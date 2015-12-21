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

#include <limits>
#include "flow_manager.h"
#include "networking_helper.h"

#include "outbound_flow.h"
#include "inbound_flow.h"

namespace libgdtp
{

FlowManager::FlowManager() :
    default_source_addr_(1),
    default_destination_addr_(BROADCAST_ADDRESS),
    default_max_seq_no_(std::numeric_limits<uint16_t>::max() - 1),
    default_ack_timeout_(100),
    default_max_num_retries_(3),
    default_buffer_size_(default_buffer_size),
    default_props_(default_props)
{
    schedulers_[DEFAULT_BELOW_PORT_ID] = SchedulerFactory::make_scheduler("fifo");
}

void FlowManager::initialize(void)
{
    // fill valid destinations
    destinations_.insert(default_source_addr_);
    destinations_.insert(BROADCAST_ADDRESS);
    destinations_.insert(BROADCAST_ADDRESS_SHORT);

    for (Addr addr : destinations_) {
        LOG_INFO("Adding " << addr << " as valid incoming address.");
    }
}

void FlowManager::deinitialize()
{
    std::map<const FlowId, std::shared_ptr<FlowBase> >::iterator it;
    for (it = flows_.begin(); it != flows_.end(); it++) {
        it->second->print_status();
    }
}

/**
 * @brief Allocate new outgoing flow
 *
 * In theorey, this method should create a new unique flow ID.
 * However, as there is no service that can determine the ID of an
 * application on the host, we need to provide the same ID
 * for the same applications on both hosts. Therefore, the
 * caller is required to propose a flow ID which is then
 * used. In future, this could be replaced by some other mechanism.
 *
 */
FlowId FlowManager::allocate_flow(const PortId dest_id, FlowProperties props)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& f : flows_) {
        if (f.second->get_dest_id() == dest_id)
            throw GdtpException("Flow already allocated.");
    }

    // create unique random source id
    FlowId src_id;
    do {
        src_id = random_generator::get_instance().uniform_0_to_n(MAX_NUM_FLOWS);
    } while (flows_.find(src_id) != flows_.end());

    // determine source address
    uint64_t src_address = default_source_addr_;
    if (props.get_addressing_mode() == IMPLICIT) {
        try
        {
            uint32_t tmp;
            NetworkingHelper::get_ipv4_address_from_dev(props.get_addr_dev(), tmp);
            src_address = tmp;
        }
        catch (std::system_error& error)
        {
            throw GdtpException("Couldn't read IPv4 address from " + props.get_addr_dev() + ": " + error.what());
        }
    }

    // creating new connection with default parameters
    LOG_INFO("Creating new outbound flow from (" << src_id << "->" << dest_id << ").");
    flows_[src_id] = std::shared_ptr<FlowBase>(new OutboundFlow(this,
                                                                              src_id,
                                                                              dest_id,
                                                                              src_address,
                                                                              default_destination_addr_,
                                                                              props,
                                                                              dest_id,
                                                                              DEFAULT_BELOW_PORT_ID,
                                                                              default_buffer_size_));

    // add source address if necessary
    if (destinations_.find(src_address) == destinations_.end()) {
        LOG_INFO("Adding " << src_address << " as valid incoming address.");
        destinations_.insert(src_address);
    }

    // creating buffers for upper layer port of this flow
    above_buffers_[dest_id].reset(new Buffer<Pdu>(default_buffer_size_));
    return src_id;
}


void FlowManager::modify_properties(const FlowId id, FlowProperties props)
{
    try {
        OutboundFlow* conn = dynamic_cast<OutboundFlow*>(flows_.at(id).get());
        conn->set_properties(props);
    } catch (const std::out_of_range& e) {
        throw GdtpException("Unknown flow specified.");
    }
}


/**
 * Determines connection that this SDU belongs to.
 * If connection doesn not exist yet, it creates one.
 * Creates a valid PDU from given SDU and adds it to the transmit queue.
 *
 * @param pdu, Frame PDU
 * @param dir, direction of this flow
 * @return string containing the id
 *
 */
int FlowManager::handle_sdu_from_above(std::shared_ptr<Data> sdu, const FlowId id)
{
    LOG_DEBUG("handle_sdu_from_above()");
    try {
        OutboundFlow* flow = dynamic_cast<OutboundFlow*>(flows_.at(id).get());
        flow->handle_frame_from_above(sdu);
    } catch (const std::out_of_range& e) {
        throw GdtpException("Unknown flow specified.");
    }
}


int FlowManager::handle_pdu_from_below(const PortId id, Pdu &pdu)
{
    LOG_DEBUG("handle_frame_from_below()");
    // check if destination address matches
    if (destinations_.find(pdu.get_dest_addr()) == destinations_.end()) {
        // frame is not for us
        LOG_DEBUG("Ignoring frame for " << pdu.get_dest_addr());
        return 0;
    }

    // get corresponding connection handle
    FlowBase* flow = find_flow(pdu, id);
    flow->handle_frame_from_below(pdu);
    return 1;
}


int FlowManager::add_frame_for_above(Pdu &pdu, const PortId id)
{
    try {
        above_buffers_.at(id)->pushBack(pdu);
    } catch (const std::out_of_range& e) {
        throw GdtpException("Unknown port id " + std::to_string(id) + " specified.");
    }
}


void FlowManager::get_pdus_for_below(PduVector &pdus, const PortId id)
{
    try {
        schedulers_.at(id)->get_pdus_for_below(pdus);
    } catch (const std::out_of_range& e) {
        throw GdtpException("Below port ID " + std::to_string(id) + " is not valid.");
    }
}


/**
 * This function is called after a frame has been transmitted on an output port.
 * It is first checked which outgoing connection is serviced on this port. If no
 * space is left in the transmission window of the ARQ protocol, we will probably
 * have to wait at this point ..
 * Call blocks.
 */
void FlowManager::set_frame_transmitted(const PortId id)
{
    try {
        schedulers_.at(id)->set_pdus_transmitted();
    } catch (const std::out_of_range& e) {
        throw GdtpException("Port id " + std::to_string(id) + " is not valid.");
    }
}


bool FlowManager::has_sdu_for_above(const PortId id)
{
    try {
        return above_buffers_.at(id)->isNotEmpty();
    } catch (const std::out_of_range& e) {
        throw GdtpException("Unknown upper layer port specified.");
    }
}


std::shared_ptr<Data> FlowManager::get_sdu_for_above(const PortId id)
{
    Pdu tmp;
    if (above_buffers_.find(id) == above_buffers_.end()) {
        std::unique_lock<std::mutex> lock(mutex_);
        above_buffers_[id].reset(new Buffer<Pdu>);
    }

    // we know id exists, so we can safely access it
    above_buffers_.at(id)->popFront(tmp);
    return tmp.get_payload();
}


FlowBase* FlowManager::find_flow(Pdu &pdu, const PortId belowid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    FlowId id;
    if (pdu.get_type() == DATA) {
        id = pdu.get_src_id();
        // check if flow already exists
        if (flows_.find(id) == flows_.end()) {
            // not found, creating new one
            LOG_INFO("Creating new inbound flow from (" << pdu.get_src_id() << "->" << pdu.get_dest_id() << ").");
            FlowProperties props = default_props_;
            // try to derive properties of new flow from existing local flow
            for (auto& f : flows_) {
               if (f.second->get_dest_id() == pdu.get_dest_id()) {
                   LOG_INFO("Using properties of existing flow.");
                   props = f.second->get_props();
               }
            }
            flows_[id] = std::shared_ptr<FlowBase>(new InboundFlow(this,
                                                                                     pdu.get_src_id(),
                                                                                     pdu.get_dest_id(),
                                                                                     pdu.get_source_addr(),
                                                                                     pdu.get_dest_addr(),
                                                                                     props,
                                                                                     pdu.get_dest_id(),
                                                                                     belowid,
                                                                                     default_buffer_size_));
            // creating buffer for upper layer port if needed
            if (above_buffers_.find(pdu.get_dest_id()) == above_buffers_.end()) {
                above_buffers_[pdu.get_dest_id()].reset(new Buffer<Pdu>(default_buffer_size_));
            }
        } else {
            // flow should be an inbound flow if it already exists
            assert(dynamic_cast<InboundFlow*>(flows_.at(id).get()) != NULL);
        }
    } else
    if (pdu.get_type() == ACK) {
        id = pdu.get_dest_id();
    }

    // finally, return flow object
    try {
        return flows_.at(id).get();
    } catch (const std::out_of_range& e) {
        throw GdtpException("Flow id " + std::to_string(id) + " is unknown.");
    }
}


void FlowManager::set_scheduler_type(const PortId port, const std::string type)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (port != DEFAULT_BELOW_PORT_ID)
        throw ParameterException("Defining a scheduler for given lower layer port is not supported at the moment.");

    schedulers_[port] = SchedulerFactory::make_scheduler(type);
}


void FlowManager::set_default_source_address(const Addr address)
{
    std::unique_lock<std::mutex> lock(mutex_);
    default_source_addr_ = address;
}


void FlowManager::set_default_destination_address(const Addr address)
{
    std::unique_lock<std::mutex> lock(mutex_);
    default_destination_addr_ = address;
}


void FlowManager::set_default_flow_properties(const FlowProperties props)
{
    std::unique_lock<std::mutex> lock(mutex_);
    default_props_ = props;
}


void FlowManager::mark_flow_as_ready(FlowBase* flow)
{
    //FIXME: acquire lock first?
    try {
        schedulers_.at(flow->get_below_port_name())->add_flow(flow);
    } catch (const std::out_of_range& e) {
        throw GdtpException("No scheduler for below port registered.");
    }
}


ArqStats FlowManager::get_stats(FlowId id)
{
    std::unique_lock<std::mutex> lock(mutex_);
    // check for outbound flows first
    if (flows_.find(id) == flows_.end()) {
        // now check inbound flows with the provided id
        for (auto& f : flows_) {
            if (dynamic_cast<InboundFlow*>(f.second.get()) != NULL) {
                if (f.second->get_dest_id() == id) {
                    // access this inbound flow via its unique src_id
                    return flows_[f.second->get_src_id()]->get_stats();
                }
            }
        }
    } else {
        return flows_[id]->get_stats();
    }
    throw GdtpException("Couldn't find flow with ID " + std::to_string(id));
}

/**
 * @brief Check if there is a PDU to be transmitted on a given lower layer port
 *
 * @param id Port identifier
 * @return true if there is a PDU for below, false otherwise
 */
bool FlowManager::has_pdu_for_below(const PortId id)
{
    try {
        return schedulers_.at(id)->has_waiting_flow();
    } catch (const std::out_of_range& e) {
        throw GdtpException("Unknown lower layer port specified.");
    }
}

ASSIGN_LOGPTR(FlowManager::logger_, FlowManager::get_name())

} // namespace libgdtp
