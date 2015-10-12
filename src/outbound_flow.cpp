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

#include "outbound_flow.h"
#include "networking_helper.h"
#include "exceptions.h"

namespace libgdtp
{

void OutboundFlow::print_status(void)
{
    ArqStats stats = get_stats(TOTAL);
    assert(stats.sdus_for_above == 0);

    LOG_INFO("  Id:                     " << src_id_);
    LOG_INFO("  Direction:              " << "Out");
    LOG_INFO("  Source:                 " << src_addr_);
    LOG_INFO("  Destination:            " << dest_addr_);
    LOG_INFO("  Serviced SDUs:          " << stats.sdus_from_above);
    LOG_INFO("  Received SDUs:          " << stats.sdus_for_above);
    LOG_INFO("  Total transm. PDUs:     " << stats.pdus_for_below);
    LOG_INFO("  Retransm. PDUs:         " << stats.rtx_pdus);
    LOG_INFO("  Lost PDUs:              " << stats.lost_pdus);
    LOG_INFO("  FER:                    " << stats.fer);
}


/**
 * @brief This function is usually called from within the main StackComponent thread.
 * The purpose of this method is to frame an incoming SDU and add it to the
 * transmit queue of the connection.
 *
 * @param the incoming SDU as StackDataSet
 */
void OutboundFlow::handle_frame_from_above(std::shared_ptr<Data> sdu)
{
    std::unique_lock<std::mutex> lock(mutex_);

#ifdef __unix__
    // overwrite addresses if needed
    if (get_props().get_addressing_mode() == IMPLICIT) {
        uint32_t src, dst;
        NetworkingHelper::get_ipv4_addresses(sdu, src, dst);
        src_addr_ = src;
        dest_addr_ = dst;
    }
#endif

    Pdu pdu(sdu);
    pdu.set_dest_addr(dest_addr_);
    pdu.set_source_addr(src_addr_);
    pdu.set_type(DATA);
    pdu.set_src_id(src_id_);
    pdu.set_dest_id(dest_id_);
    pdu.set_seq_no(get_next_seq_no());
    if (is_broadcast()) {
        // send broadcast frames directly without ARQ
        queue_pdu_for_below(pdu);
    } else {
        // only handle acknowledeged connections with ARQ
        arq_->handle_pdu_from_above(pdu);
    }
}


void OutboundFlow::handle_frame_from_below(Pdu& pdu)
{
    if (is_broadcast()) {
        throw GdtpException("Getting PDU from below on unacknowleged outbound connection.");
    }

    // pass PDU to arq instance
    arq_->handle_pdu_from_below(pdu);
}


void OutboundFlow::frame_transmitted(void)
{
    LOG_DEBUG("frame_transmitted()");
    if (not is_broadcast()) {
        // may have to wait for ACK
        arq_->frame_transmitted();
    }
}


SeqNo OutboundFlow::get_next_seq_no(void)
{
    return ((++seq_no_) % get_props().get_max_seqno());
}


ASSIGN_LOGPTR(OutboundFlow::logger_, OutboundFlow::get_name())

} // namespace libgdtp
