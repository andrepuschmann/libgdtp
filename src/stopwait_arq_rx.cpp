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

#include "stopwait_arq_rx.h"
#include "flow_base.h"
#include "inbound_flow.h"

namespace libgdtp {

StopWaitArqRx::StopWaitArqRx(FlowBase* flow, size_t buffer_size) :
    ArqBase(flow, buffer_size),
    last_seq_no_(0),
    expected_seq_no_(0)
{}

void StopWaitArqRx::handle_pdu_from_below(Pdu& pdu)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    if (pdu.get_type() != DATA)
        throw GdtpException("Invalid frame received on this flow.");

    handle_data_pdu(pdu);
    stats_.pdus_from_below++;
}

void StopWaitArqRx::handle_data_pdu(Pdu& pdu)
{
    LOG_DEBUG("handle_data_pdu()");
    bool valid_frame = true;
    SeqNo seq_no = pdu.get_seq_no();
    SeqNo ack_no = seq_no; // this is the seqno that will be acknowledged, if any

    // we have to consider a number of cases in which we accept (or discard) the frame ..
    // - seq_no == expected_seq_no_, which is the default case
    // - seq_no == last_seq_no, duplicate
    // - expected_seq_no_ == 0, this either indicates a receiver startup or a seqno wrap-up
    //   in the startup case, we would accept anything, but not if it was a wrap-up
    // - seq_no > expected_seq_no, this indicates a frame loss, but we accept the new frame
    // - seq_no < expected_seq_no, this indicates an old frame, so the receiver may have not received the previous ack
    //   however, we might have simply missed the seqno wrapup. in this case, we may accept
    //   seqnos that are smaller than half of the maximum seqno window in order to make sure to not discard
    //   even more frames unnecessarily
    if (seq_no == last_seq_no_) {
        LOG_INFO("Duplicate frame received.");
        valid_frame = false;
    } else
    if (seq_no < expected_seq_no_ && seq_no > get_props().get_max_seqno() / 2) {
        LOG_INFO("Old frame received (expected " << expected_seq_no_ << ").");
        valid_frame = false;
        ack_no = (expected_seq_no_ == 0) ? 0 : expected_seq_no_ - 1; // send ack for previous PDU
    } else
    if (seq_no > expected_seq_no_ && expected_seq_no_ != 0) {
        uint32_t num_lost = seq_no - expected_seq_no_;
        LOG_INFO("Future frame received, lost " << num_lost << " frames.");
        stats_.lost_pdus += num_lost;
    }

    // return ACK if flow is reliable
    if (get_props().get_transfer_mode() == RELIABLE) {
        Pdu ack = get_ack_for_data_frame(pdu, ack_no);
        LOG_INFO("Transmitting ACK " << ack.get_seq_no());
        flow_->queue_pdu_for_below(ack);
        stats_.pdus_for_below++;
    }

    // pass frame to upper layer
    if (valid_frame) {
        assert(seq_no != last_seq_no_);
        flow_->queue_pdu_for_above(pdu);
        expected_seq_no_ = (ack_no + 1) % get_props().get_max_seqno();
        stats_.sdus_for_above++;
        stats_.bytes_for_above += pdu.get_payload()->size();
    }

    last_seq_no_ = seq_no;
}

Pdu StopWaitArqRx::get_ack_for_data_frame(Pdu &data, const SeqNo seqno)
{
    Pdu ack;
    ack.set_type(ACK);
    ack.set_dest_addr(data.get_source_addr());
    ack.set_source_addr(data.get_dest_addr());
    ack.set_src_id(data.get_dest_id());
    ack.set_dest_id(data.get_src_id());
    ack.set_seq_no(seqno);
    return ack;
}

ASSIGN_LOGPTR(StopWaitArqRx::logger_, StopWaitArqRx::get_name())

}
