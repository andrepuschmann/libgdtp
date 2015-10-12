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

#ifndef GDTPIMPL_H
#define GDTPIMPL_H

#include "libgdtp.h"
#include "flow_manager.h"

namespace libgdtp
{

class Gdtp::GdtpImpl
{
public:
    GdtpImpl();
    ~GdtpImpl();

    void initialize();
    void set_scheduler_type(const PortId port, const std::string type);
    void set_default_source_address(const Addr source);
    void set_default_destination_address(const Addr destination);
    int allocate_flow(const FlowId id, FlowProperties props);
    void modify_properties(const FlowId id, FlowProperties props);

    bool has_data_for_below(const FlowId id);
    void get_data_for_below(const FlowId id, Data& data);
    void handle_data_from_below(const PortId id, Data& data);
    void set_data_transmitted(const FlowId id);

    void handle_data_from_above(std::shared_ptr<Data> data, const FlowId id);
    bool has_data_for_above(const FlowId id);
    std::shared_ptr<Data> get_data_for_above(const FlowId id);

    FlowStats get_stats(const FlowId id);

private:
    GdtpImpl(const GdtpImpl&); // non-copyable ('= delete' in C++11)
    GdtpImpl& operator=(const GdtpImpl&); // non-copyable ('= delete' in C++11)
    static std::string get_name(void) { return "GdtpImpl"; }

    std::unique_ptr<FlowManager> manager_;
    EncoderStats stats_;

    DECLARE_LOGPTR(logger_)
};

} // namespace libgdtp

#endif // GDTPIMPL_H
