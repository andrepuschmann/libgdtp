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

/*! \brief This class implements outgoing flows.
 *
 * This class implements outgoing flows, i.e., flows that have their origin
 * in this protocol instance and are destined to another node.
 */

#ifndef OUTBOUND_FLOW_H
#define OUTBOUND_FLOW_H

#include "flow_base.h"
#include "stopwait_arq_tx.h"

namespace libgdtp
{

class OutboundFlow : public FlowBase
{
public:
    OutboundFlow(FlowManager* manager,
                       const FlowId src_id,
                       const FlowId dest_id,
                       Addr source,
                       Addr destination,
                       FlowProperties props,
                       PortId above_port_name,
                       PortId below_port_name,
                       size_t buffer_size)
        : FlowBase(manager,
                         src_id,
                         dest_id,
                         source,
                         destination,
                         props,
                         above_port_name,
                         below_port_name,
                         OUTGOING,
                         buffer_size),
          seq_no_(0)
    {
        arq_ = std::unique_ptr<StopWaitArqTx>(new StopWaitArqTx(this, buffer_size));
    }
    ~OutboundFlow() {}

    void handle_frame_from_above(std::shared_ptr<Data> Data);
    void handle_frame_from_below(Pdu& pdu);
    void print_status(void);
    void frame_transmitted(void);

private:
    // member functions
    SeqNo get_next_seq_no(void);
    static std::string get_name(void) { return "OutboundFlow"; }

    // member variables
    std::atomic<SeqNo> seq_no_;
    Buffer<Pdu> buffer_from_above_;
    DECLARE_LOGPTR(logger_)
};

} // namespace libgdtp

#endif // OUTBOUND_FLOW_H
