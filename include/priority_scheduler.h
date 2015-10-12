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

#ifndef PRIORITY_SCHEDULER_H
#define PRIORITY_SCHEDULER_H

#include "scheduler_base.h"
#include <stdio.h>
#include <queue>

namespace libgdtp
{

// Provide external compare function so the priority-queue knows
// how to compare two flows with one another
// Note that we don't overload the less-than operator of flow_base
// to allow other implementation, too
struct priority_compare
{
    bool operator()(FlowBase* t1, FlowBase* t2) const
    {
        return (t1->get_priority() > t2->get_priority());
    }
};

class PriorityScheduler : public SchedulerBase
{
public:
    PriorityScheduler() {}
    void add_flow(FlowBase*);
    bool has_waiting_flow(void);
    void get_pdus_for_below(PduVector &pdus);

private:
    FlowBase* get_next_flow();
    std::string getName(void) { return "PriorityScheduler"; }

    // This queue holds pointers to all connections that are ready
    // to be served ordered according to their priority
    std::priority_queue<FlowBase*, std::vector<FlowBase*>, priority_compare> queue_;
};

} // namespace libgdtp

#endif // PRIORITY_SCHEDULER_H
