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

#include "flow_base.h"
#include "flow_manager.h"

namespace libgdtp
{

void FlowBase::queue_pdu_for_below(Pdu& pdu)
{
    buffer_for_below_.pushBack(pdu);
    manager_->mark_flow_as_ready(this);
}

void FlowBase::queue_pdu_for_above(Pdu& pdu)
{
    manager_->add_frame_for_above(pdu, above_port_name_);
}

} // namespace libgdtp
