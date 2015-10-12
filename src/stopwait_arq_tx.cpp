/* -*- c++ -*- */
/*
 * Copyright 2013-2015, André Puschmann <andre.puschmann@tu-ilmenau.de>
 *
 * This file is part of libgdtp.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "stopwait_arq_tx.h"
#include "flow_base.h"
#include "inbound_flow.h"

namespace libgdtp {

StopWaitArqTx::StopWaitArqTx(FlowBase* flow, size_t buffer_size) :
    ArqBase(flow, buffer_size),
    tx_seq_no_(0)
{
    // start transmit thread
    arq_thread_ = boost::thread(&StopWaitArqTx::arq_thread_function, this);
}


StopWaitArqTx::~StopWaitArqTx(void)
{
    tx_cond_.notify_one();
    timer_cond_.notify_one();
    arq_thread_.interrupt();
    arq_thread_.join();
}


void StopWaitArqTx::arq_thread_function(void)
{
    try {
        while (true) {
            boost::this_thread::interruption_point();
            Pdu tmp;
            buffer_.popFront(tmp);
            transmit_pdu(tmp);
        }
    }
    catch(boost::thread_interrupted)
    {
        LOG_INFO("ARQ thread " << boost::this_thread::get_id() << " was interrupted.");
    }
}


void StopWaitArqTx::transmit_pdu(Pdu& pdu)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    tx_seq_no_ = pdu.get_seq_no();
    LOG_DEBUG("Selecting PDU " << tx_seq_no_ << " for " << pdu.get_dest_addr() << " for transmission.");

    int num_tx = 0;
    state_ = IDLE;
    while (state_ != FINISHED) {
        // enqueue frame
        flow_->queue_pdu_for_below(pdu);
        state_ = WAITING_FOR_TX;
        stats_.pdus_for_below++;
        num_tx++;

        // waiting for actual transmission
        LOG_DEBUG("Waiting for " << num_tx << ". transmission.");
        while (state_ != TX_OVER) {
            tx_cond_.wait(lock);
        }
        state_ = WAITING_FOR_ACK;

        // finish transmission of this PDU in the unreliable case
        if (get_props().get_transfer_mode() == UNRELIABLE)
            return;

        // waiting for ACK
        LOG_DEBUG("Waiting for ACK ..");
        boost::system_time const timeout = boost::get_system_time() +
                            boost::posix_time::milliseconds(get_props().get_ack_timeout());
        while (state_ == WAITING_FOR_ACK) {
            if (timer_cond_.timed_wait(lock, timeout) == false) {
                state_ = TIMEOUT;
            }
        }

        switch (state_) {
        case TIMEOUT:
            LOG_DEBUG("ACK timeout.");
            if (num_tx >= get_props().get_max_retransmission()) {
                LOG_INFO("Frame lost, maximum number of retransnmissions reached.");
                state_ = FINISHED;
                stats_.lost_pdus++;
            } else {
                stats_.rtx_pdus++;
            }
            break;
        case FINISHED:
            LOG_DEBUG("Got ACK!");
            break;
        default:
            throw GdtpException("Invalid state");
        }
    }
}


void StopWaitArqTx::handle_pdu_from_below(Pdu& pdu)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    if (pdu.get_type() != ACK)
        throw GdtpException("Invalid frame received on this flow.");

    handle_ack_pdu(pdu);
    stats_.pdus_from_below++;
}


void StopWaitArqTx::handle_ack_pdu(Pdu& pdu)
{
    LOG_DEBUG("handle_ack_pdu()");
    if (state_ == WAITING_FOR_ACK) {
        uint32_t seq_no = pdu.get_seq_no();
        LOG_DEBUG("pdu seq no: " << seq_no);
        LOG_DEBUG("tx seq no: " << tx_seq_no_);

        if (seq_no == tx_seq_no_) {
            LOG_DEBUG("Received correct ack.");
            state_ = FINISHED;
            timer_cond_.notify_one();
        } else if (seq_no < tx_seq_no_) {
            LOG_DEBUG("Received old ack.");
        } else {
            LOG_ERROR("Received future ACK.");
        }
    } else {
        LOG_INFO("Ignoring ACK.");
    }
}


void StopWaitArqTx::frame_transmitted()
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    assert(state_ == WAITING_FOR_TX);
    state_ = TX_OVER;
    tx_cond_.notify_one();
}

ASSIGN_LOGPTR(StopWaitArqTx::logger_, StopWaitArqTx::get_name())

}
