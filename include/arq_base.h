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

#ifndef ARQ_BASE_H
#define ARQ_BASE_H

#include <iostream>
#include <cstdint>
#include <assert.h>
#include <thread>
#include <mutex>
#include "exceptions.h"
#include "logger.h"
#include "buffer.h"
#include "pdu.h"

namespace libgdtp {

#define NON_BLOCKING_ARQ 0

typedef enum
{
    IDLE = 0,
    WAITING_FOR_TX,
    TX_OVER,
    WAITING_FOR_ACK,
    FINISHED,
    TIMEOUT
} ArqState;

typedef enum
{
    RUNNING = 0,
    TOTAL
} StatsMode;

class FlowBase;

class ArqBase : boost::noncopyable
{
    friend class StopWaitArqRx;
    friend class StopWaitArqTx;
public:
    /**
     * The constructor of the base class requires to pass
     * the creating flow in order to enqueue outgoing frames.
     */
    ArqBase(FlowBase* const flow, const size_t buffer_size);

    virtual ~ArqBase() {}

    virtual void handle_pdu_from_above(Pdu& pdu);

    virtual void handle_pdu_from_below(Pdu& pdu) = 0;

    virtual void frame_transmitted() = 0;

    ArqStats get_stats(StatsMode mode);

private:
    // member functions
    const FlowProperties get_props(void);
    static std::string get_name(void) { return "ArqBase"; }

    // private variables
    FlowBase* const flow_;
    ArqState state_;
    Buffer<Pdu> buffer_;
    boost::thread arq_thread_;
    ArqStats stats_ = {};
    ArqStats last_stats_ = {};
    boost::mutex mutex_;

    DECLARE_LOGPTR(logger_)
};

}

#endif // ARQ_BASE_H
