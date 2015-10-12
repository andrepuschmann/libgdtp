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

#include <iostream>
#include <memory>
#include <stdint.h>
#include "logger.h"
#include "gdtpimpl.h"
#include "protobuf_codec.h"

namespace libgdtp
{

Gdtp::GdtpImpl::GdtpImpl() :
    manager_(new FlowManager())
{
    LOG_DEBUG("Constructing GdtpImpl ..");
    CONFIG_LOGGER();
}


Gdtp::GdtpImpl::~GdtpImpl()
{
    LOG_INFO("Deconstructing GDTP ..");
    manager_->deinitialize();
    LOG_INFO("  Bytes for below:     " << stats_.bytes_for_below);
    LOG_INFO("  Bytes from above:    " << stats_.bytes_from_above);
    LOG_INFO("  Bytes for above:     " << stats_.bytes_for_above);
    LOG_INFO("  Bytes from below:    " << stats_.bytes_from_below);
    LOG_INFO("  Frames from below:   " << stats_.frames_from_below);
    LOG_INFO("  Frames for below:    " << stats_.frames_for_below);
    google::protobuf::ShutdownProtobufLibrary();
    DESTROY_LOGGER()
}


void Gdtp::GdtpImpl::initialize()
{
    LOG_INFO("Initializing GDTP ..");
    manager_->initialize();
}


void Gdtp::GdtpImpl::set_scheduler_type(const PortId port, const std::string type)
{
    manager_->set_scheduler_type(port, type);
}


void Gdtp::GdtpImpl::set_default_source_address(const Addr addr)
{
    manager_->set_default_source_address(addr);
}


void Gdtp::GdtpImpl::set_default_destination_address(const Addr addr)
{
    manager_->set_default_destination_address(addr);
}


int Gdtp::GdtpImpl::allocate_flow(const FlowId id, FlowProperties props)
{
    return manager_->allocate_flow(id, props);
}


void Gdtp::GdtpImpl::modify_properties(const FlowId id, FlowProperties props)
{
    manager_->modify_properties(id, props);
}


void Gdtp::GdtpImpl::handle_data_from_above(std::shared_ptr<Data> sdu, const FlowId id)
{
    stats_.bytes_from_above += sdu->size();
    manager_->handle_sdu_from_above(sdu, id);
}


void Gdtp::GdtpImpl::handle_data_from_below(const PortId id, Data& sdu)
{
    // decode lower layer SDUs into PDU
    PduVector pdus;
    ProtobufCodec::decode(sdu, pdus);
    stats_.bytes_from_below += sdu.size();
    stats_.frames_from_below++;

    for (Pdu i : pdus) {
        LOG_INFO("RX " << i.get_type_as_string() << " " << i.get_seq_no() << " from " << i.get_source_addr());
        manager_->handle_pdu_from_below(id, i);
    }
}


bool Gdtp::GdtpImpl::has_data_for_below(const PortId id)
{
    return manager_->has_pdu_for_below(id);
}


void Gdtp::GdtpImpl::get_data_for_below(const FlowId id, Data& data)
{
    // get PDUs
    PduVector pdus;
    manager_->get_pdus_for_below(pdus, id);

    // encode PDU into Protobuf OTA format
    ProtobufCodec::encode(pdus, data);

    for (auto& p : pdus) {
        LOG_INFO("TX " << p.get_type_as_string() << " " << p.get_seq_no() << " to " << p.get_dest_addr());
    }
    stats_.bytes_for_below += data.size();
    stats_.frames_for_below++;
}


void Gdtp::GdtpImpl::set_data_transmitted(const FlowId id)
{
    manager_->set_frame_transmitted(id);
}


bool Gdtp::GdtpImpl::has_data_for_above(const FlowId id)
{
    return manager_->has_sdu_for_above(id);
}


std::shared_ptr<Data> Gdtp::GdtpImpl::get_data_for_above(const FlowId id)
{
    std::shared_ptr<Data> sdu = manager_->get_sdu_for_above(id);
    stats_.bytes_for_above += sdu->size();
    return sdu;
}


FlowStats Gdtp::GdtpImpl::get_stats(const FlowId id)
{
    FlowStats flow_stats;
    flow_stats.arq = manager_->get_stats(id); // get arq stats from manager first
    flow_stats.encoder = stats_;
    return flow_stats;
}


ASSIGN_LOGPTR(Gdtp::GdtpImpl::logger_, Gdtp::GdtpImpl::get_name())

} // namespace libgdtp
