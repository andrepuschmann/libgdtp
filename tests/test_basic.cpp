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

#define BOOST_TEST_MODULE Basic_test

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "libgdtp.h"
#include "outbound_flow.h"
#include "exceptions.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

#define DEFAULT_ID 1

BOOST_AUTO_TEST_SUITE(Basic_test)

BOOST_AUTO_TEST_CASE(Create_test)
{
    BOOST_REQUIRE_NO_THROW(Gdtp tx());
}


BOOST_AUTO_TEST_CASE(Init_test)
{
    Gdtp prot;
    prot.set_scheduler_type("fifo");
    prot.set_default_source_address(1);
    prot.set_default_destination_address(2);
    prot.initialize();
}


BOOST_AUTO_TEST_CASE(Call_test)
{
    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.initialize();

    // create flow with default requirements
    FlowId id = prot.allocate_flow(DEFAULT_ID);

    // prot should always return false as no frame is to be sent
    for (int i = 0; i < 10; i++) {
        BOOST_CHECK(prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == false);
    }

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> in_buffer = make_shared<Data>(10, 0xff);
    prot.handle_data_from_above(in_buffer, id);
    //BOOST_REQUIRE_THROW(prot.handle_frame_from_above(in_buffer, out_port), GdtpException);

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    for (int i = 0; i < 10; i++) {
        BOOST_CHECK(prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);
    }
}


BOOST_AUTO_TEST_CASE(Input_test)
{
    Gdtp prot;
    prot.set_default_source_address(0);
    prot.set_default_destination_address(1);
    prot.initialize();

    // create flow with default requirements
    FlowId id = prot.allocate_flow(DEFAULT_ID);

    // create dummy upper layer SDU and feed it to GDTP instance
    std::shared_ptr<Data> in_buffer = make_shared<Data>(10, 0xff);
    prot.handle_data_from_above(in_buffer, id);

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));;

    // run transmitter once
    BOOST_CHECK(prot.has_data_for_below(DEFAULT_BELOW_PORT_ID) == true);

    // Loop until connection is serviced
    while (prot.has_data_for_below(DEFAULT_BELOW_PORT_ID)) {
        Data frame;
        prot.get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);
        std::cout << "Got outgoing frame for port " << DEFAULT_BELOW_PORT_ID << " with size: " << frame.size() << std::endl;
        // call send function here and signal frame transmitted ..
        prot.set_data_transmitted(DEFAULT_BELOW_PORT_ID);
    }
}


BOOST_AUTO_TEST_CASE(SeqNo_test)
{
    const PortId above_port(1);
    const PortId below_port(1);
    FlowProperties props;
    FlowId src_id(1);
    FlowId dest_id(2323);
    std::unique_ptr<OutboundFlow> flow(new OutboundFlow(NULL,
                                                        src_id,
                                                        dest_id,
                                                        1,
                                                        2,
                                                        props,
                                                        above_port,
                                                        below_port,
                                                        1));
}

BOOST_AUTO_TEST_SUITE_END()
