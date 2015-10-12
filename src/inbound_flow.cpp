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

#include "inbound_flow.h"
#include "flow_manager.h"

namespace libgdtp
{

void InboundFlow::print_status(void)
{
    ArqStats stats = get_stats(TOTAL);
    assert(stats.sdus_from_above == 0);

    LOG_INFO("  Id:                     " << src_id_);
    LOG_INFO("  Direction:              " << "In");
    LOG_INFO("  Source:                 " << src_addr_);
    LOG_INFO("  Destination:            " << dest_addr_);
    LOG_INFO("  Total transm. PDUs:     " << stats.pdus_for_below);
    LOG_INFO("  Received PDUs:          " << stats.sdus_for_above);
    LOG_INFO("  Lost PDUs:              " << stats.lost_pdus);
    LOG_INFO("  FER:                    " << stats.fer);
}

void InboundFlow::handle_frame_from_below(Pdu& pdu)
{
    // if broadcast, pass up directly
    if (is_broadcast()) {
        queue_pdu_for_above(pdu);
    } else {
        arq_->handle_pdu_from_below(pdu);
    }
}

void InboundFlow::frame_transmitted(void)
{
    LOG_DEBUG("frame_transmitted()");
}

ASSIGN_LOGPTR(InboundFlow::logger_, InboundFlow::get_name())

} // namespace libgdtp
