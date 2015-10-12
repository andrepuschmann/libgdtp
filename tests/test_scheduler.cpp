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

#define BOOST_TEST_MODULE Scheduler_test

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;


#define DEFAULT_ID 1
#define DEFAULT_ACK_TIMEOUT 100
#define PAYLOAD_SIZE_SHORT 10
#define PAYLOAD_SIZE_LONG 100

BOOST_AUTO_TEST_SUITE(Scheduler_test)

BOOST_AUTO_TEST_CASE(fifo_scheduler_test)
{
    FlowProperties props;
    props.set_transfer_mode(UNRELIABLE);

    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.set_scheduler_type("fifo");
    prot.initialize();

    // create flow with default properties
    FlowId id1 = prot.allocate_flow(1, props);

    // create second flow with higher priority
    uint32_t prio = props.get_priority();
    props.set_priority(prio--); // increase priority
    FlowId id2 = prot.allocate_flow(2, props);

    // create two SDUs
    std::shared_ptr<Data> short_sdu = make_shared<Data>(PAYLOAD_SIZE_SHORT, 0xff);
    std::shared_ptr<Data> long_sdu = make_shared<Data>(PAYLOAD_SIZE_LONG, 0xff);

    // hand both SDUs to libgdtp
    prot.handle_data_from_above(short_sdu, id1);
    prot.handle_data_from_above(long_sdu, id2);

    // sleep and give libgdtp a chance to queue the frame
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    // run transmitter once, this should be the shorter SDU
    Data frame;
    prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);
    prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);
    BOOST_CHECK(frame.size() < PAYLOAD_SIZE_LONG);

    // run the second time, this should be the larger SDU
    prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);
    prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);
    BOOST_CHECK(frame.size() > PAYLOAD_SIZE_LONG);

    // there shouldn't be any pending frames left
    BOOST_CHECK(prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
}

BOOST_AUTO_TEST_CASE(priority_scheduler_test)
{
    FlowProperties props;
    props.set_transfer_mode(UNRELIABLE);

    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.set_scheduler_type("priority");
    prot.initialize();

    // create flow with default properties
    FlowId id1 = prot.allocate_flow(1, props);

    // create second flow with higher priority
    uint32_t prio = props.get_priority();
    props.set_priority(--prio); // increase priority
    FlowId id2 = prot.allocate_flow(2, props);

    // create two SDUs
    std::shared_ptr<Data> short_sdu = make_shared<Data>(PAYLOAD_SIZE_SHORT, 0xff);
    std::shared_ptr<Data> long_sdu = make_shared<Data>(PAYLOAD_SIZE_LONG, 0xff);

    // hand both SDUs to libgdtp
    prot.handle_data_from_above(short_sdu, id1);
    prot.handle_data_from_above(long_sdu, id2);

    // sleep and give libgdtp a chance to queue the frame
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    // run transmitter once, this should be the shorter SDU
    Data frame;
    prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);
    prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);
    BOOST_CHECK(frame.size() > PAYLOAD_SIZE_LONG);

    // run the second time, this should be the larger SDU
    prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);
    prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);
    BOOST_CHECK(frame.size() < PAYLOAD_SIZE_LONG);

    // there shouldn't be any pending frames left
    BOOST_CHECK(prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
}


BOOST_AUTO_TEST_CASE(implicitack_scheduler_test)
{
    FlowProperties props;

    Gdtp tx_prot;
    tx_prot.set_default_source_address(1);
    tx_prot.set_default_destination_address(2);
    tx_prot.initialize();

    Gdtp rx_prot;
    rx_prot.set_default_source_address(2);
    rx_prot.set_default_destination_address(1);
    rx_prot.set_scheduler_type("implicitack");
    rx_prot.initialize();

    // create flow with default requirements
    FlowId id1 = tx_prot.allocate_flow(DEFAULT_ID, props);
    FlowId id2 = rx_prot.allocate_flow(DEFAULT_ID + 1, props);

    // create dummy upper layer SDU and feed it to both GDTP instances
    std::shared_ptr<Data> data = make_shared<Data>(10, 0xff);
    tx_prot.handle_data_from_above(data, id1);
    rx_prot.handle_data_from_above(data, id2);

    boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    // run transmitter once
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    Data data1;
    tx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, data1);
    rx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, data1);
    tx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

    // check if data can be passed to upper layer at the receiver
    BOOST_CHECK(rx_prot.has_data_for_above(DEFAULT_ID) == true);
    std::shared_ptr<Data> sdu_for_upper_layer = rx_prot.get_data_for_above(DEFAULT_ID);

    // run receiver once
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    // get ack and data frame and feed into transmitter
    Data pdu;
    rx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, pdu);
    tx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, pdu);
    rx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

    // now check for second
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);
    tx_prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, pdu);
    rx_prot.handle_data_from_below(DEFAULT_BELOW_PORT_ID, pdu);
    tx_prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);

    // check if data can be passed to upper layer at the receiver
    BOOST_CHECK(tx_prot.has_data_for_above(DEFAULT_ID + 1) == true);
    sdu_for_upper_layer = tx_prot.get_data_for_above(DEFAULT_ID + 1);

    // there shouldn't be any other pending frames
    BOOST_CHECK(tx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
    BOOST_CHECK(rx_prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);

    int num_frames = 0;
    FlowStats stats = tx_prot.get_stats(id1);
    num_frames += stats.encoder.frames_for_below;
    stats = rx_prot.get_stats(id2);
    num_frames += stats.encoder.frames_for_below;
    BOOST_CHECK(num_frames == 3);
}

BOOST_AUTO_TEST_SUITE_END()

