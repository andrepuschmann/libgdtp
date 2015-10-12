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

#define BOOST_TEST_MODULE Codec_test

#include <boost/test/unit_test.hpp>
#include "libgdtp.h"
#include "protobuf_codec.h"
#include "exceptions.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Codec_test)

BOOST_AUTO_TEST_CASE(ProtobufCodec_test)
{
    // create a dummy PDU
    std::shared_ptr<Data> payload = make_shared<Data>(10, 0xff);
    Pdu outgoing_pdu(payload);
    outgoing_pdu.set_source_addr(1);
    outgoing_pdu.set_dest_addr(2);
    outgoing_pdu.set_type(DATA);
    outgoing_pdu.set_src_id(1);
    outgoing_pdu.set_dest_id(99);
    outgoing_pdu.set_seq_no(1);

    // create dummy PDU vector
    PduVector pdus;
    pdus.push_back(outgoing_pdu);

    // create an empty lower layer SDU and encode PDU
    std::shared_ptr<Data> frame = make_shared<Data>();
    ProtobufCodec::encode(pdus, *frame.get());

    // create an empty PDU and decode lower layer SDU into it
    pdus.clear();
    ProtobufCodec::decode(*frame.get(), pdus);
    BOOST_CHECK(pdus.size() == 1);

    // verify incoming PDU equals outgoing PDU
    BOOST_CHECK(pdus.front().get_source_addr() == outgoing_pdu.get_source_addr());
    BOOST_CHECK(pdus.front().get_dest_addr() == outgoing_pdu.get_dest_addr());
}


BOOST_AUTO_TEST_CASE(ProtobufEncoderSmallNetwork_test)
{
    // create a dummy PDU
    std::shared_ptr<Data> payload = make_shared<Data>(10, 0xff);
    Pdu outgoing_pdu(payload);
    outgoing_pdu.set_source_addr(1);
    outgoing_pdu.set_dest_addr(2);
    outgoing_pdu.set_type(DATA);
    outgoing_pdu.set_src_id(3);
    outgoing_pdu.set_dest_id(4);
    outgoing_pdu.set_seq_no(5);

    // create dummy PDU vector
    PduVector pdus;
    pdus.push_back(outgoing_pdu);

    // create an empty lower layer SDU and encode PDU
    std::shared_ptr<Data> frame = make_shared<Data>();
    ProtobufCodec::encode(pdus, *frame.get());

    BOOST_CHECK(frame->size() == 26);
}


BOOST_AUTO_TEST_CASE(ProtobufEncoderBigNetwork_test)
{
    // create a dummy PDU
    std::shared_ptr<Data> payload = make_shared<Data>(10, 0xff);
    Pdu outgoing_pdu(payload);
    outgoing_pdu.set_source_addr(4294967294);
    outgoing_pdu.set_dest_addr(4294967295);
    outgoing_pdu.set_type(DATA);
    outgoing_pdu.set_src_id(4294967293);
    outgoing_pdu.set_dest_id(4294967294);
    outgoing_pdu.set_seq_no(4294967292);

    // create dummy PDU vector
    PduVector pdus;
    pdus.push_back(outgoing_pdu);

    // create an empty lower layer frame and encode PDU
    std::shared_ptr<Data> frame = make_shared<Data>();
    ProtobufCodec::encode(pdus, *frame.get());

    BOOST_CHECK(frame->size() == 46);
}

BOOST_AUTO_TEST_SUITE_END()
