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

#ifndef INBOUND_FLOW_H
#define INBOUND_FLOW_H

#include "flow_base.h"
#include "stopwait_arq_rx.h"

namespace libgdtp
{

class InboundFlow : public FlowBase
{
public:
    InboundFlow(FlowManager* manager,
                      FlowId src_id,
                      FlowId dest_id,
                      uint64_t source,
                      uint64_t destination,
                      FlowProperties props,
                      PortId above_port_name,
                      PortId below_port_name,
                      const size_t buffer_size)
        : FlowBase(manager,
                         src_id,
                         dest_id,
                         source,
                         destination,
                         props,
                         above_port_name,
                         below_port_name,
                         INBOUND,
                         buffer_size)
    {
        arq_ = std::unique_ptr<StopWaitArqRx>(new StopWaitArqRx(this, buffer_size));
    }
    ~InboundFlow() {}
    void print_status(void);
    void handle_frame_from_below(Pdu& pdu);
    void frame_transmitted(void);

private:
    // member functions
    static std::string get_name(void) { return "InboundFlow"; }

    // member variables
    DECLARE_LOGPTR(logger_)
};

} // namespace libgdtp

#endif // INBOUND_FLOW_H
