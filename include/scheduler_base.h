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

#ifndef SCHEDULER_BASE_H
#define SCHEDULER_BASE_H

#include "flow_base.h"
#include <unordered_set>

namespace libgdtp
{

class SchedulerBase : boost::noncopyable
{
public:
    SchedulerBase() :
        flow_(NULL)
    {}
    virtual ~SchedulerBase() {}
    virtual void add_flow(FlowBase*) = 0;
    virtual bool has_waiting_flow(void) = 0;
    virtual void get_pdus_for_below(PduVector &pdus) = 0;
    void set_pdus_transmitted(void);

protected:
    // This set holds the connection ids that are currently serviced by the scheduler
    // This allow O(1) lookups if new connections get signalled
    std::unordered_set<FlowId> serviced_flows_;
    boost::mutex mutex_;
    boost::condition_variable not_empty_cond_;
    FlowBase* flow_; // holds the active flow

private:
    virtual FlowBase* get_next_flow() = 0;
};

} // namespace libgdtp

#endif // SCHEDULER_BASE_H
