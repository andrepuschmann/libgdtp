#
# Copyright 2013-2015, André Puschmann <andre.puschmann@tu-ilmenau.de>
#
# This file is part of libgdtp.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

SET(headers
    libgdtp.h
    gdtpimpl.h
    arq_base.h
    stopwait_arq_tx.h
    stopwait_arq_rx.h
    flow_base.h
    flow_manager.h
    flow_property.h
    outbound_flow.h
    inbound_flow.h
    scheduler_base.h
    scheduler_factory.h
    fifo_scheduler.h
    priority_scheduler.h
    implicitack_scheduler.h
    pdu.h
    logger.h
    exceptions.h
    buffer.h
    protobuf_codec.h
    networking_helper.h
    random_generator.h
)

# adding dummy target in order to introduce headers to coding IDE (e.g., QtCreator)
add_custom_target(dummy SOURCES ${headers})

# Install header files
install(FILES
    libgdtp.h
    flow_property.h
    DESTINATION include/libgdtp
)
