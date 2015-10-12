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

#ifndef STOPWAIT_ARQ_TX_H
#define STOPWAIT_ARQ_TX_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include "arq_base.h"
#include "logger.h"

namespace libgdtp {

class FlowBase;

class StopWaitArqTx : public ArqBase
{
public:
    StopWaitArqTx(FlowBase* flow, size_t buffer_size);
    ~StopWaitArqTx(void);

    void handle_pdu_from_below(Pdu& pdu);
    void frame_transmitted();

private:
    // member functions defined in arq_base
    void arq_thread_function(void);
    void handle_ack_pdu(Pdu& pdu);

    void transmit_pdu(Pdu& pdu);
    static std::string get_name(void) { return "StopWaitArqTx"; }

    // member variables ..
    std::atomic<uint32_t> tx_seq_no_; // seqno of currently served PDU

    // timer synchronization and rtx handling
    boost::condition_variable timer_cond_;
    boost::condition_variable tx_cond_;
    DECLARE_LOGPTR(logger_)
};

}

#endif // STOPWAIT_ARQ_TX_H
