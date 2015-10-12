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


# Prevent in-tree builds
IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "Prevented in-tree built. This is bad practice.")
ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# Project setup
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(LIBGDTP)
ENABLE_TESTING()
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Add includes and dependencies
FIND_PACKAGE(Protobuf REQUIRED)

# Setup Boost
MESSAGE(STATUS "")
MESSAGE(STATUS "Configuring Boost C++ Libraries...")
SET(BOOST_REQUIRED_COMPONENTS
    date_time
    filesystem
    program_options
    system
    thread
    unit_test_framework
)

FIND_PACKAGE(Boost 1.37 REQUIRED ${BOOST_REQUIRED_COMPONENTS})
MESSAGE(STATUS "Boost version: ${Boost_VERSION}")

IF(Boost_VERSION LESS 104600)
    ADD_DEFINITIONS( -DBOOST_FILESYSTEM_VERSION=2 ) #use filesystem version 2 in boost < 1.46
    MESSAGE(STATUS "Using Boost Filesystem V2")
ENDIF(Boost_VERSION LESS 104600)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

MESSAGE(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "Boost library directories: ${Boost_LIBRARY_DIRS}")
MESSAGE(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# Setup Log4cxx
MESSAGE(STATUS "")
MESSAGE(STATUS "Configuring Log4cxx...")
#SET(LOG4CPP_FIND_REQUIRED TRUE)
FIND_PACKAGE(Log4cxx REQUIRED)

# Make sure the compiler can find include files
INCLUDE_DIRECTORIES(
    ${LIBGDTP_SOURCE_DIR}/include
    ${LOG4CPP_INCLUDE_DIRS}
)

# Enable C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# adding subdirectories
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(tests)

# Make sure the linker can find the library once it is built.
LINK_DIRECTORIES(${LIBGDTP_BINARY_DIR}/src)
