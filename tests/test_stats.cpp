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

#define BOOST_TEST_MODULE Stats_test

#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;


#define DEFAULT_ID 1
#define ACK_TIMEOUT 2000

BOOST_AUTO_TEST_SUITE(Stats_test)

BOOST_AUTO_TEST_CASE(BasicFer_test)
{
    FlowProperties props;
    props.set_transfer_mode(UNRELIABLE);

    Gdtp tx_prot;
    tx_prot.set_default_source_address(1);
    tx_prot.set_default_destination_address(2);
    tx_prot.initialize();

    Gdtp rx_prot;
    rx_prot.set_default_source_address(2);
    rx_prot.initialize();

    // create flows with identical requirements
    FlowId tx_id = tx_prot.allocate_flow(DEFAULT_ID, props);
    FlowId rx_id = rx_prot.allocate_flow(DEFAULT_ID, props);

    for (int i = 0; i < 5; i++) {
        // create dummy upper layer SDU and feed it to GDTP instance
        std::shared_ptr<Data> sdu = make_shared<Data>(10 + i, 0xff);
        tx_prot.handle_data_from_above(sdu, tx_id);

        boost::this_thread::sleep(boost::posix_time::milliseconds(50));

        // run transmitter
        Data data1;
        BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);
        tx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, data1);
        tx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

        // feed frame to rx entity
        if (i != 1)
            rx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, data1);
    }

    FlowStats stats = rx_prot.get_stats(DEFAULT_ID);
    BOOST_CHECK_CLOSE(stats.arq.fer, 0.2f, 0.01 );
}

BOOST_AUTO_TEST_SUITE_END()
