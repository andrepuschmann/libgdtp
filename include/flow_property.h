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

/*! \brief FlowProperties define the characteristics of a flow.
 *
 * FlowProperties define the characteristics of a flow, such as
 * the degree of reliability or its priority.
 */

#ifndef FLOW_PROPERTY_H
#define FLOW_PROPERTY_H

#include <string>
#include <stdint.h>

namespace libgdtp
{

typedef enum {
    RELIABLE = 0,
    UNRELIABLE
} TransferMode;


typedef enum
{
    EXPLICIT = 0,
    IMPLICIT
} AddressingMode;


class FlowProperties
{
public:
    FlowProperties(TransferMode transfer_mode = RELIABLE,
                 uint32_t priority = 99,
                 AddressingMode addressing_mode = EXPLICIT,
                 uint64_t max_seqno = 127,
                 uint32_t ack_timeout = 100,
                 uint32_t max_retransmission = 7,
                 std::string addr_dev = "tun0"):
        transfer_mode_(transfer_mode),
        priority_(priority),
        addressing_mode_(addressing_mode),
        max_seqno_(max_seqno),
        ack_timeout_(ack_timeout),
        max_retransmission_(max_retransmission),
        addr_dev_(addr_dev)
    {}

    TransferMode get_transfer_mode() const { return transfer_mode_; }
    uint32_t get_priority() const { return priority_; }
    AddressingMode get_addressing_mode() const { return addressing_mode_; }
    uint64_t get_max_seqno() const { return max_seqno_; }
    uint32_t get_max_retransmission() const { return max_retransmission_; }
    uint32_t get_ack_timeout() const { return ack_timeout_; }
    std::string get_addr_dev() const { return addr_dev_; }
    std::string pp_string() const
    {
        switch (transfer_mode_) {
        case RELIABLE: return "Reliable"; break;
        case UNRELIABLE: return "Unreliable"; break;
        default: return "UNKNOWN";
        }
    }
    void set_transfer_mode(const TransferMode mode) { transfer_mode_ = mode; }
    void set_priority(const uint32_t prio) { priority_ = prio; }
    void set_addressing_mode(const AddressingMode mode) { addressing_mode_ = mode; }
    void set_max_seqno(const uint32_t no) { max_seqno_ = no; }
    void set_ack_timeout(const uint32_t timeout) { ack_timeout_ = timeout; }
    void set_max_retransmissions(const uint32_t no) { max_retransmission_ = no; }
    void set_addr_dev(const std::string dev) { addr_dev_ = dev; }

private:
    TransferMode transfer_mode_;
    uint32_t priority_;
    /* Not implemented:
    int throughput;
    int delay;
    */
    AddressingMode addressing_mode_;
    uint64_t max_seqno_;
    uint32_t ack_timeout_;
    uint32_t max_retransmission_;
    std::string addr_dev_;
};

} // namespace libgdtp

#endif // FLOW_PROPERTY_H

