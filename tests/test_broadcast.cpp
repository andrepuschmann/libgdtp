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

#define BOOST_TEST_MODULE Broadcast_test

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

#define DEFAULT_ID 1

BOOST_AUTO_TEST_SUITE(Broadcast_test)

BOOST_AUTO_TEST_CASE(BroadcastTransmit_test)
{
    Gdtp prot;
    prot.set_default_source_address(1);
    prot.set_default_destination_address(127); // default broadcast address for small networks
    prot.initialize();

    // create flow with default requirements
    FlowId id = prot.allocate_flow(DEFAULT_ID);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> in_buffer = make_shared<Data>(10, 0xff);
    prot.handle_data_from_above(in_buffer, id);

    // for broadcast frames, there should be no retransmission
    while (prot.has_data_for_below(DEFAULT_BELOW_PORT_ID)) {
        Data frame;
        prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);

        // Pass frame down to lower layer
        std::cout << "Tx frame for port " << DEFAULT_BELOW_PORT_ID << " with size: " << frame.size() << std::endl;
        prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

        // Wait for some large retransmit delay ... which will not cause another frame to be sent
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }

    FlowStats stats = prot.get_stats(id);
    BOOST_CHECK(stats.encoder.frames_for_below == 1);
}

BOOST_AUTO_TEST_SUITE_END()

