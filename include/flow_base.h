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

#ifndef FLOW_BASE_H
#define FLOW_BASE_H

#include <stdio.h>
#include <atomic>
#include <boost/lexical_cast.hpp>
#include "buffer.h"
#include "logger.h"
#include "pdu.h"
#include "flow_property.h"
#include "arq_base.h"

namespace libgdtp
{

#define BROADCAST_ADDRESS 65535
#define BROADCAST_ADDRESS_SHORT 127

typedef enum
{
    INBOUND = 0,
    OUTGOING,
} FlowDirection;

// forward declaration
class FlowManager;

class FlowBase : boost::noncopyable
{
    friend class OutboundFlow;
    friend class InboundFlow;
public:
    FlowBase(FlowManager* manager,
                   const FlowId src_id,
                   const FlowId dest_id,
                   const Addr src_addr,
                   const Addr dest_addr,
                   FlowProperties props,
                   const PortId above_port_name,
                   const PortId below_port_name,
                   const FlowDirection direction,
                   uint32_t buffer_size) :
        manager_(manager),
        src_id_(src_id),
        dest_id_(dest_id),
        src_addr_(src_addr),
        dest_addr_(dest_addr),
        properties_(props),
        above_port_name_(above_port_name),
        below_port_name_(below_port_name),
        direction_(direction),
        buffer_for_below_(buffer_size)
    {
    }
    virtual ~FlowBase() {}

    virtual void print_status(void) = 0;

    virtual void handle_frame_from_below(Pdu& pdu) = 0;

    virtual void frame_transmitted(void) = 0;

    ArqStats get_stats(StatsMode mode = RUNNING) { return arq_->get_stats(mode); }

    /**
     * @brief Return relative priority of this connection (the larger, the higher).
     * @return int, the priority.
     */
    uint32_t get_priority(void) { return properties_.get_priority(); }

    /**
     * @brief Return whether connection is broadcast or not.
     * @return true if this is a broadcast connection, false otherwise.
     */
    bool is_broadcast(void)
    {
        return ((dest_addr_ == BROADCAST_ADDRESS_SHORT) ||
                (dest_addr_ == BROADCAST_ADDRESS) ? true : false);
    }

    /**
     * @brief Return whether connection is reliable or not.
     * @return true if this connection should be error free, false otherwise.
     */
    bool is_unreliable(void) { return (properties_.get_transfer_mode() == UNRELIABLE); }

    /**
     * @brief has_frame_for_below() checks whether the below buffer of this
     * flow is empty or not.
     * @return true if queue contains an element, false otherwise
     */
    bool has_frame_for_below(void) { return buffer_for_below_.isNotEmpty(); }

    /**
     * @brief get_frame_for_below() is a blocking method that pops the first element from
     * its queue and returns it to the caller.
     * The mutex_ must NOT be hold when calling the method (causes deadlock with writer)
     */
    void get_frame_for_below(Pdu &pdu) { buffer_for_below_.popFront(pdu); }

    /**
     * @brief Return the current output portname of the connection.
     * @return the outputport name
     */
    PortId get_below_port_name(void) { return below_port_name_; }

    /**
     * @brief Return the flows' port ID at the source
     * @return the id
     */
    uint32_t get_src_id(void) { return src_id_; }

    /**
     * @brief Return the flows' port ID at the destination
     * @return the id
     */
    uint32_t get_dest_id(void) { return dest_id_; }

    /**
     * @brief Return the size of the outbound buffer.
     * @return the current length
     */
    size_t get_below_buffer_size(void) { return buffer_for_below_.size(); }

    const FlowProperties get_props(void) { return properties_; }

    void set_properties(FlowProperties props)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        properties_ = props;
    }

    FlowDirection get_direction(void) { return direction_; }

    void queue_pdu_for_above(Pdu& pdu);

    void queue_pdu_for_below(Pdu& pdu);

private:
    // member variables
    const FlowId src_id_;
    const FlowId dest_id_;
    Addr src_addr_;
    Addr dest_addr_;
    FlowProperties properties_;
    const PortId above_port_name_;
    const PortId below_port_name_;
    const FlowDirection direction_;

    std::unique_ptr<ArqBase> arq_;
    Buffer<Pdu> buffer_for_below_;

    FlowManager* manager_;
    std::mutex mutex_;
};

} // namespace libgdtp

#endif // FLOW_BASE_H
