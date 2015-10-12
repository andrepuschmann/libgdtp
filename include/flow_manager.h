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

/*! \brief The FlowManager is a core element that handles all outbound and
 *         inbound flows.
 *
 * The FlowManager is a core element that handles all outbound and
 * inbound flows. The FlowManager also handles the scheduler for each outbound interface
 * (e.g. for each PHY interace) as well as the interfaces for each upper
 * layer port (i.e., for libgdtp clients).
 */

#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

#include <map>
#include <iostream>
#include <string>
#include <stdio.h>
#include <boost/thread/mutex.hpp>
#include <boost/cast.hpp>
#include <random>
#include "logger.h"
#include "exceptions.h"
#include "flow_base.h"
#include "random_generator.h"
#include "scheduler_factory.h"
#ifdef __unix__
#include "networking_helper.h"
#endif

namespace libgdtp
{

const size_t default_buffer_size = 10;

class FlowManager : boost::noncopyable
{
public:
    FlowManager();
    ~FlowManager() {}
    void initialize(void);
    void deinitialize(void);
    void set_scheduler_type(const PortId port, const std::string type);
    void set_default_source_address(const Addr address);
    void set_default_destination_address(const Addr address);
    void set_default_flow_properties(const FlowProperties props);

    FlowId allocate_flow(const PortId id, FlowProperties props);
    void modify_properties(const FlowId id, FlowProperties props);
    void mark_flow_as_ready(FlowBase* flow);

    ArqStats get_stats(FlowId id);

    int handle_sdu_from_above(std::shared_ptr<Data> sdu, const FlowId id);
    int handle_pdu_from_below(const PortId id, Pdu &pdu);
    int add_frame_for_above(Pdu &pdu, const FlowId id);

    bool has_pdu_for_below(const PortId id);
    void get_pdus_for_below(PduVector &pdus, const FlowId id);
    void set_frame_transmitted(const PortId id);

    bool has_sdu_for_above(const PortId id);
    std::shared_ptr<Data> get_sdu_for_above(const PortId id);

private:
    FlowBase* find_flow(Pdu &pdu, const PortId belowid);
    static std::string get_name(void) { return "FlowManager"; }

    // member variables
    Addr default_source_addr_;
    Addr default_destination_addr_;
    uint64_t default_max_seq_no_;
    uint32_t default_ack_timeout_;
    uint32_t default_max_num_retries_;
    uint32_t default_buffer_size_;
    FlowProperties default_props_;

    std::set<Addr> destinations_; ///< Addresses to listen for in incoming frames (search may have linear complexity)
    std::map<const FlowId, std::shared_ptr<FlowBase> > flows_; ///< FIXME: may replace with more efficient data structures for search and insert
    std::map<PortId, std::unique_ptr<SchedulerBase> > schedulers_; ///< A map containing the actual schedulers
    std::map<PortId, std::unique_ptr<Buffer<Pdu> > > above_buffers_; ///< A map containing the data buffers for each upper layer port
    std::mutex mutex_;

    DECLARE_LOGPTR(logger_)
};

} // namespace libgdtp

#endif // FLOW_MANAGER_H
