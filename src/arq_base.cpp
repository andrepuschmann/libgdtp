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

#include "arq_base.h"
#include "flow_base.h"

namespace libgdtp
{

ArqBase::ArqBase(FlowBase* const flow, const size_t buffer_size) :
    flow_(flow),
    state_(IDLE),
    buffer_(buffer_size),
    stats_()
{
}

void ArqBase::handle_pdu_from_above(Pdu &pdu)
{
#if NON_BLOCKING_ARQ
    if (buffer_.capacity() != buffer_.size()) {
#endif
        buffer_.pushBack(pdu);
        stats_.sdus_from_above++;
        stats_.bytes_from_above += pdu.get_payload()->size();
#if NON_BLOCKING_ARQ
    } else {
        LOG_DEBUG("arq buffer full, dropping frame");
    }
#endif
}

const FlowProperties ArqBase::get_props(void)
{
    return flow_->get_props();
}

ArqStats ArqBase::get_stats(StatsMode mode)
{
    ArqStats tmp = stats_;
    int frames_for_above = stats_.sdus_for_above;
    int lost_frames = stats_.lost_pdus;
    if (mode == RUNNING) {
        frames_for_above -= last_stats_.sdus_for_above;
        lost_frames -= last_stats_.lost_pdus;
    }

    int total_frames = frames_for_above + lost_frames;
    if (total_frames > 0) {
        tmp.fer = lost_frames / static_cast<float>(total_frames);
    }

    last_stats_ = stats_;
    return tmp;
}

ASSIGN_LOGPTR(ArqBase::logger_, ArqBase::get_name())

} // namespace libgdtp
