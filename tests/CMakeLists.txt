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

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)

MACRO(ADD_UNIT_TEST name)
  ADD_EXECUTABLE(test_${name} test_${name}.cpp ${ARGN})
  INCLUDE_DIRECTORIES(${LIBGDTP_BINARY_DIR}/src) # for Protobuf generated files
  INCLUDE_DIRECTORIES(${LIBGDTP_SOURCE_DIR}/src) # for Protobuf generated files
  TARGET_LINK_LIBRARIES(test_${name} gdtp ${LOG4CXX_LIBRARIES} ${Boost_LIBRARIES})
  ADD_TEST(${name} ${EXECUTABLE_OUTPUT_PATH}/test_${name})
ENDMACRO()

ADD_UNIT_TEST(basic)
ADD_UNIT_TEST(broadcast)
ADD_UNIT_TEST(codec)
ADD_UNIT_TEST(misc)
ADD_UNIT_TEST(stopwait_arq)
ADD_UNIT_TEST(requirements)
ADD_UNIT_TEST(scheduler)
ADD_UNIT_TEST(stats)
ADD_UNIT_TEST(tuntap)
ADD_UNIT_TEST(unicast)
