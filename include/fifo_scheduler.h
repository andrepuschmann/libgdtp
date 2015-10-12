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

#ifndef FIFO_SCHEDULER_H
#define FIFO_SCHEDULER_H

#include "scheduler_base.h"
#include "logger.h"
#include <stdio.h>

namespace libgdtp
{

class FifoScheduler : public SchedulerBase
{
public:
    FifoScheduler() {}
    void add_flow(FlowBase* flow);
    void get_pdus_for_below(PduVector &pdus);
    bool has_waiting_flow(void);

private:
    FlowBase* get_next_flow();
    static std::string get_name(void) { return "FifoScheduler"; }

    // This queue holds pointers to all connections that are ready
    // to be served in the order they have been inserted (FIFO)
    std::queue<FlowBase*> queue_;
    DECLARE_LOGPTR(logger_)
};

} // namespace libgdtp

#endif // FIFO_SCHEDULER_H
