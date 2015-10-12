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

#define BOOST_TEST_MODULE Unicast_test

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

#define DEFAULT_ID 1
#define DEFAULT_ACK_TIMEOUT 100
#define MAX_TRANSMISSIONS 3
#define PAYLOAD_SIZE 10

BOOST_AUTO_TEST_SUITE(unicast_test)

BOOST_AUTO_TEST_CASE(reliable_test)
{
    FlowProperties props;
    props.set_ack_timeout(DEFAULT_ACK_TIMEOUT);
    props.set_max_retransmissions(MAX_TRANSMISSIONS);

    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.initialize();

    // create reliable flow
    FlowId id = prot.allocate_flow(DEFAULT_ID, props);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> in_buffer = make_shared<Data>(PAYLOAD_SIZE, 0xff);
    prot.handle_data_from_above(in_buffer, id);

    // sleep and give libgdtp a chance to queue the frame
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    // run transmitter
    while (prot.has_data_for_below(DEFAULT_BELOW_PORT_ID)) {
        Data frame;
        prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);

        // Pass frame down to lower layer
        std::cout << "Tx frame on flow " << id << " with size: " << frame.size() << std::endl;
        prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

        // wait here at least for ACK timeout duration
        boost::this_thread::sleep(boost::posix_time::milliseconds(100 + DEFAULT_ACK_TIMEOUT));
    }
    FlowStats stats = prot.get_stats(id);
    BOOST_CHECK(stats.arq.pdus_for_below == MAX_TRANSMISSIONS);
    BOOST_CHECK(stats.arq.bytes_from_above == PAYLOAD_SIZE);
}

BOOST_AUTO_TEST_CASE(unreliable_test)
{
    FlowProperties props(UNRELIABLE);

    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.initialize();

    // create unreliable flow
    FlowId id = prot.allocate_flow(DEFAULT_ID, props);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> in_buffer = make_shared<Data>(PAYLOAD_SIZE, 0xff);
    prot.handle_data_from_above(in_buffer, id);

    // sleep and give libgdtp a chance to queue the frame
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    // run transmitter
    while (prot.has_data_for_below(DEFAULT_BELOW_PORT_ID)) {
        Data frame;
        prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);

        // .. passing frame down to lower layer ..
        std::cout << "Tx frame on flow " << id << " with size: " << frame.size() << std::endl;
        prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

        // wait here at least for ACK timeout duration
        boost::this_thread::sleep(boost::posix_time::milliseconds(100 + DEFAULT_ACK_TIMEOUT));
    }
    FlowStats stats = prot.get_stats(id);
    BOOST_CHECK(stats.arq.pdus_for_below == 1);
    BOOST_CHECK(stats.arq.bytes_from_above == PAYLOAD_SIZE);
}

BOOST_AUTO_TEST_SUITE_END()
