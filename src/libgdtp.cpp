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

#include "libgdtp.h"
#include "gdtpimpl.h"

namespace libgdtp
{

// forwards:
Gdtp::Gdtp() : impl_(new GdtpImpl) {}
Gdtp::~Gdtp() {} // required by 'std::unique_ptr'!

Gdtp::Gdtp(Gdtp&& rhs) : impl_(std::move(rhs.impl_)) {}
Gdtp& Gdtp::operator=(Gdtp&& rhs)
{
    impl_ = std::move(rhs.impl_);
    return *this;
}

void Gdtp::initialize()
{
    impl_->initialize();
}

void Gdtp::set_scheduler_type(const std::string type, const PortId port)
{
    impl_->set_scheduler_type(port, type);
}

void Gdtp::set_default_source_address(const Addr source)
{
    impl_->set_default_source_address(source);
}

void Gdtp::set_default_destination_address(const Addr destination)
{
    impl_->set_default_destination_address(destination);
}

int Gdtp::allocate_flow(const FlowId id, FlowProperties props)
{
    return impl_->allocate_flow(id, props);
}

void Gdtp::handle_data_from_above(std::shared_ptr<Data> sdu, const FlowId id)
{
    impl_->handle_data_from_above(sdu, id);
}

void Gdtp::handle_data_from_below(const PortId id, Data& data)
{
    impl_->handle_data_from_below(id, data);
}

void Gdtp::set_data_transmitted(const FlowId id)
{
    impl_->set_data_transmitted(id);
}

bool Gdtp::has_data_for_above(const FlowId id)
{
    return impl_->has_data_for_above(id);
}

std::shared_ptr<Data> Gdtp::get_data_for_above(const FlowId id)
{
    return impl_->get_data_for_above(id);
}

bool Gdtp::has_data_for_below(const PortId id)
{
    return impl_->has_data_for_below(id);
}

void Gdtp::get_data_for_below(const FlowId id, Data& data)
{
    impl_->get_data_for_below(id, data);
}

void Gdtp::modify_properties(const FlowId id, const FlowProperties props)
{
    impl_->modify_properties(id, props);
}

FlowStats Gdtp::get_stats(const FlowId id)
{
    return impl_->get_stats(id);
}

} // namespace libgdtp
