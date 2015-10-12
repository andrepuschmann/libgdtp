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

# Build the library from source files
SET(sources
    libgdtp.cpp
    gdtpimpl.cpp
    arq_base.cpp
    flow_base.cpp
    flow_manager.cpp
    outbound_flow.cpp
    inbound_flow.cpp
    fifo_scheduler.cpp
    priority_scheduler.cpp
    implicitack_scheduler.cpp
    arq_base.cpp
    stopwait_arq_tx.cpp
    stopwait_arq_rx.cpp
    scheduler_base.cpp
    gdtp.pb.cc
)

# Targets must be globally unique for cmake
ADD_LIBRARY(gdtp SHARED ${sources})
TARGET_LINK_LIBRARIES(gdtp ${PROTOBUF_LIBRARIES} ${LOG4CXX_LIBRARIES})

LINK_DIRECTORIES(${LOG4CPP_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) # needed for Protobuf generated files

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS gdtp.proto)

# Adding install targets
INSTALL(TARGETS gdtp DESTINATION lib)
