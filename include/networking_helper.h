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

/*! \brief This class provides helper methods related to networking.
 *
 * This header provides helper methods for accessing the network stack of
 * the operating systems as well as easier handling of network packets.
 * This includes retrieving the current address of a networking device as
 * well as low-level parsing of network packets.
 */

#ifndef NETWORKING_HELPER_H
#define NETWORKING_HELPER_H

#include <boost/shared_ptr.hpp>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "libgdtp.h"
#include "exceptions.h"

namespace libgdtp
{

class NetworkingHelper
{
public:
    /**
     * \brief Parse payload and retrieve Ethernet source and destionation addresses.
     *
     * This function parses the provided chunk of data, interprets it as an Ethernet frame and tries
     * to retrieve the source and destionation addresses of it.
     *
     * \param packet the data
     * \param source reference to the source address to be filled
     * \param destination reference to the destination address to be filled
     */
    static void get_ipv4_addresses(std::shared_ptr<Data> sdu, uint32_t &source, uint32_t &destination)
    {
        assert(sdu->size() > sizeof(struct ip));
        const struct ether_header* ethernet_header = (struct ether_header*)sdu->data();;
        const struct ip* ip_header;

        // try to find ethernet header first (tap device)
        if ((ntohs(ethernet_header->ether_type) == ETHERTYPE_IP) &&
            (ntohs(ethernet_header->ether_type) == ETHERTYPE_ARP))
        {
            // this is an Ethernet frame, adjust ip header
            ip_header = (struct ip*)(ethernet_header + sizeof(struct ether_header));
        } else {
            // this appears to be an IP frame already
            ip_header = (struct ip*)ethernet_header;
        }

        if (ip_header->ip_v != 4) {
            throw GdtpException("Couldn't read IPv4 addresses from frame.");
        }
        source = ip_header->ip_src.s_addr;
        destination = ip_header->ip_dst.s_addr;
    }

    /**
     * \brief Obtain the IPv4 address assigned to a given networking device.
     *
     * This function tries to obtain the IPv4 address assigned to a given
     * networking device but doesn't convert it from its internal representation
     * as a uint32_t.
     *
     * \param dev the network device (i.e. eth0)
     * \param addr reference to the address to be filled
     */
    static void get_ipv4_address_from_dev(const std::string dev, uint32_t &addr)
    {
        struct ifreq ifr;
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET; // IPv4 address
        strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ - 1);
        if (ioctl(s, SIOCGIFADDR, &ifr) != 0) {
            throw std::system_error(errno, std::system_category());
        }
        close(s);

        // in_addr_t.s_addr is uint32_t
        struct in_addr ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
        addr = static_cast<uint32_t>(ip.s_addr);
    }

    /**
     * \brief Get the IPv4 address of a given networking device as a string.
     *
     * This function tries to obtain the IPv4 address and returns it as
     * as std::string.
     *
     * \param dev the network device (i.e. eth0)
     * \param addr reference to a string object to be filled with the address
     */
    static void get_ipv4_address_from_dev(const std::string dev, std::string &addr)
    {
        uint32_t raw_addr;
        struct in_addr ip;
        get_ipv4_address_from_dev(dev, raw_addr);
        ip.s_addr = static_cast<in_addr_t>(raw_addr);
        addr.assign(inet_ntoa(ip), strlen(inet_ntoa(ip)));
    }
};

} // namespace libgdtp

#endif // NETWORKING_HELPER_H
