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

#ifndef PDU_H
#define PDU_H

#include "libgdtp.h"

namespace libgdtp
{

class Pdu;

typedef std::vector<Pdu> PduVector;

enum Type { DATA, ACK, BROADCAST };

class Pdu
{
public:
    Pdu() :
        payload_(new Data),
        type_(DATA),
        seqno_(0)
    {}

    Pdu(std::shared_ptr<Data> sdu) :
        payload_(sdu),
        type_(DATA),
        seqno_(0)
    {}

    Data* get_payload_ptr(void)
    {
        return payload_.get();
    }

    std::shared_ptr<Data> get_payload(void)
    {
        return payload_;
    }

    FlowId get_src_id(void)
    {
        return src_id_;
    }

    FlowId get_dest_id(void)
    {
        return dest_id_;
    }

    Addr get_source_addr(void)
    {
        return source_addr_;
    }

    Addr get_dest_addr(void)
    {
        return dest_addr_;
    }

    SeqNo get_seq_no(void)
    {
        return seqno_;
    }

    Type get_type(void)
    {
        return type_;
    }

    std::string get_type_as_string(void)
    {
        switch (type_) {
        case DATA:
            return "DATA";
            break;
        case ACK:
            return "ACK";
            break;
        default:
            return "UNKNOWN";
        }
    }

    void set_source_addr(const Addr addr)
    {
        source_addr_ = addr;
    }

    void set_dest_addr(const Addr addr)
    {
        dest_addr_ = addr;
    }

    void set_seq_no(const SeqNo no)
    {
        seqno_ = no;
    }

    void set_type(const Type type)
    {
        type_ = type;
    }

    void set_src_id(const FlowId id)
    {
        src_id_ = id;
    }

    void set_dest_id(const FlowId id)
    {
        dest_id_ = id;
    }

private:
    FlowId src_id_;
    FlowId dest_id_;
    Addr source_addr_;
    Addr dest_addr_;
    Type type_;
    SeqNo seqno_;
    std::shared_ptr<Data> payload_;
};

} // namespace libgdtp

#endif // PDU_H
