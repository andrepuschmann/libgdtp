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

#include "priority_scheduler.h"

namespace libgdtp
{

void PriorityScheduler::add_flow(FlowBase* flow)
{
    boost::lock_guard<boost::mutex> lock(mutex_);
    // only add connections to queue if they are not serviced yet
    if (serviced_flows_.find(flow->get_src_id()) == serviced_flows_.end()) {
        serviced_flows_.insert(flow->get_src_id());
        queue_.push(flow);
    }
    not_empty_cond_.notify_one();
}


FlowBase* PriorityScheduler::get_next_flow()
{
    boost::mutex::scoped_lock lock(mutex_);
    while (queue_.empty()) {
        not_empty_cond_.wait(lock);
    }
    flow_ = queue_.top();
    queue_.pop();
    serviced_flows_.erase(flow_->get_src_id());
    return flow_;
}


bool PriorityScheduler::has_waiting_flow(void)
{
    boost::mutex::scoped_lock lock(mutex_);
    return (not queue_.empty());
}


void PriorityScheduler::get_pdus_for_below(PduVector &pdus)
{
    this->get_next_flow();
    assert(flow_ != NULL);

    Pdu pdu;
    flow_->get_frame_for_below(pdu);
    pdus.push_back(pdu);
}

} // namespace libgdtp
