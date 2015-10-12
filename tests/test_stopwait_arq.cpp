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

#define BOOST_TEST_MODULE StopWaitArq_test

#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

#define DEFAULT_ID 1
#define ACK_TIMEOUT 2000

BOOST_AUTO_TEST_SUITE(StopWaitArq_test)

BOOST_AUTO_TEST_CASE(Reliable_test)
{
    FlowProperties props;

    Gdtp tx_prot;
    tx_prot.set_default_source_address(1);
    tx_prot.set_default_destination_address(2);
    tx_prot.initialize();

    Gdtp rx_prot;
    rx_prot.set_default_source_address(2);
    rx_prot.initialize();

    // create flow with default requirements
    FlowId id = tx_prot.allocate_flow(DEFAULT_ID, props);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> sdu = make_shared<Data>(10, 0xff);
    tx_prot.handle_data_from_above(sdu, id);

    boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    // receive protocol instance shouldn't have a waiting conn
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);

    // run transmitter once
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    Data data1;
    tx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, data1);

    // feed frame to rx entity
    rx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, data1);

    // Tell tx that frame has been transmitted
    tx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

    // check if data can be passed to upper layer at the receiver
    BOOST_CHECK(rx_prot.has_data_for_above(DEFAULT_ID) == true);
    std::shared_ptr<Data> sdu_for_upper_layer = rx_prot.get_data_for_above(DEFAULT_ID);

    // run receiver once
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    // get ack frame and feed into transmitter
    Data ack;
    rx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, ack);
    tx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, ack);

    // there shouldn't be any other pending frames
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
}

BOOST_AUTO_TEST_CASE(Unreliable_test)
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

    // create unreliable flow on both sides
    FlowId id = tx_prot.allocate_flow(DEFAULT_ID, props);
    rx_prot.allocate_flow(DEFAULT_ID, props);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> sdu = make_shared<Data>(10, 0xff);
    tx_prot.handle_data_from_above(sdu, id);

    boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    // receive protocol instance shouldn't have a waiting conn
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);

    // run transmitter once
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    Data data1;
    tx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, data1);

    // feed frame to rx entity
    rx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, data1);

    // Tell tx that frame has been transmitted
    tx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

    // check if data can be passed to upper layer at the receiver
    BOOST_CHECK(rx_prot.has_data_for_above(DEFAULT_ID) == true);
    std::shared_ptr<Data> sdu_for_upper_layer = rx_prot.get_data_for_above(DEFAULT_ID);

    // there shouldn't be any other pending frames
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
}

BOOST_AUTO_TEST_SUITE_END()
