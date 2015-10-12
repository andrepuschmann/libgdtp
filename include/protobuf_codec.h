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

#ifndef PROTOBUF_ENCODER_H
#define PROTOBUF_ENCODER_H

#include <boost/lexical_cast.hpp>
#include "libgdtp.h"
#include "pdu.h"
#include "gdtp.pb.h"
#include "exceptions.h"

namespace libgdtp
{

class ProtobufCodec
{
public:
    /**
     * Encode an outgoing PDU into a valid lower layer SDU.
     *
     * \param pdu Reference to a Pdu object
     * \param lower_layer_sdu Reference to a Sdu object
     * \return void
     */
    static void encode(PduVector& pdus, Data& lower_layer_sdu)
    {
        GdtpFrame frame;

        for (Pdu i : pdus) {
            GdtpPdu* protopdu = frame.add_pdu();
            try {
                protopdu->set_source(boost::lexical_cast<uint64_t>(i.get_source_addr()));
                protopdu->set_destination(boost::lexical_cast<uint64_t>(i.get_dest_addr()));
                protopdu->set_src_id(boost::lexical_cast<uint32_t>(i.get_src_id()));
                protopdu->set_dest_id(boost::lexical_cast<uint32_t>(i.get_dest_id()));
                protopdu->set_seqno(i.get_seq_no());
            }
            catch( const boost::bad_lexical_cast & ) {
                throw GdtpException("Error while converting protocol fields.");
            }

            switch (i.get_type())
            {
            default:
            case DATA:
                protopdu->set_type(GdtpPdu::DATA);
                break;
            case BROADCAST:
                protopdu->set_type(GdtpPdu::BROADCAST);
                break;
            case ACK:
                protopdu->set_type(GdtpPdu::ACK);
                break;
            }
            // copy payload from shared object
            protopdu->add_payload(&i.get_payload_ptr()->front(), i.get_payload_ptr()->size());

            // serialize
            lower_layer_sdu.resize(frame.ByteSize());
            frame.SerializeWithCachedSizesToArray(&lower_layer_sdu.front());
        }
    }


    /**
     * Decode an incoming SDU from a lower layer into a valid PDU of
     * this protocol.
     *
     * \param lower_layer_sdu Reference to a Sdu object
     * \param pdu Reference to a Pdu object
     * \return void
     */
    static void decode(Data& lower_layer_sdu, PduVector& pdus)
    {
        GdtpFrame frame;
        if (not frame.ParseFromArray((const void*)&lower_layer_sdu.front(), lower_layer_sdu.size())) {
            throw GdtpException("Decoding frame failed.");
        }

        size_t num_pdus = frame.pdu_size();
        for (int i = 0; i < num_pdus; i++) {
            Pdu pdu;
            GdtpPdu* protopdu = frame.mutable_pdu(i);

            // construct control information
            pdu.set_source_addr(protopdu->source());
            pdu.set_dest_addr(protopdu->destination());
            pdu.set_src_id(protopdu->src_id());
            pdu.set_dest_id(protopdu->dest_id());
            pdu.set_seq_no(protopdu->seqno());
            switch (protopdu->type())
            {
            default:
            case GdtpPdu::DATA:
                pdu.set_type(DATA);
                break;
            case GdtpPdu::BROADCAST:
                pdu.set_type(BROADCAST);
                break;
            case GdtpPdu::ACK:
                pdu.set_type(ACK);
                break;
            }

            // copy payload into provided buffer
            assert(protopdu->mutable_payload()->size() == 1);
            std::string payload = protopdu->mutable_payload()->Get(0);
            pdu.get_payload_ptr()->clear();
            pdu.get_payload_ptr()->insert(pdu.get_payload_ptr()->end(), payload.c_str(), payload.c_str() + payload.size());
            pdus.push_back(pdu);
        }
    }
};

} // namespace libgdtp

#endif // PROTOBUF_ENCODER_H
