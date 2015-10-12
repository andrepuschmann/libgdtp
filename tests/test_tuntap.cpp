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

#define BOOST_TEST_MODULE TunTap_test

#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <boost/format.hpp>

#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

#define MAX_BUF_SIZE (10 * 1024) // should be enough for now

#define DEFAULT_ID 1
#define ACK_TIMEOUT 2000
#define TUN_DEV "tun0"

/**
  * Byte representation of a ping request from 10.0.0.1 to 10.0.0.2
  * Length is 84 Byte
  */
static const uint8_t ping_request[] = {
    0x45, 0x00, 0x00, 0x54, 0xa4, 0x81, 0x40, 0x00,
    0x40, 0x01, 0x82, 0x25, 0x0a, 0x00, 0x00, 0x01,
    0x0a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x80, 0x82,
    0x34, 0x61, 0x00, 0x40, 0xd8, 0x0c, 0x19, 0x55,
    0x00, 0x00, 0x00, 0x00, 0x86, 0xa7, 0x0c, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37
};





BOOST_AUTO_TEST_SUITE(TunTap_test)

/* Arguments taken by the function:
*
* code shamelessly taken from
* http://backreference.org/2010/03/26/tuntap-interface-tutorial/
*
* char *dev: the name of an interface (or '\0'). MUST have enough
*   space to hold the interface name if '\0' is passed
* int flags: interface flags (eg, IFF_TUN etc.)
*/
int allocate(char *dev, int flags)
{
  struct ifreq ifr;
  int fd, err;
  const char *clonedev = (const char *)"/dev/net/tun";


  // open the clone device
  if((fd = open(clonedev, O_RDWR)) < 0)
  {
    return fd;
  }

  // preparation of the struct ifr, of type "struct ifreq"
  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;   // IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI

  if (*dev)
  {
    // if a device name was specified, put it in the structure; otherwise,
    // the kernel will try to allocate the "next" device of the
    // specified type
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  // try to create the device
  if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
  {
    close(fd);
    return err;
  }

  // if the operation was successful, write back the name of the
  // interface to the variable "dev", so the caller can know
  // it. Note that the caller MUST reserve space in *dev (see calling
  // code below)
  strcpy(dev, ifr.ifr_name);

  // this is the special file descriptor that the caller will use to talk
  // with the virtual interface
  return fd;
}


BOOST_AUTO_TEST_CASE(TunAlloc_test)
{
    // open TUN device
    char dev[IFNAMSIZ] = TUN_DEV;
    int flags = IFF_TUN | IFF_NO_PI;
    int fd = allocate(dev, flags);
    BOOST_CHECK(fd >= 0);
    close(fd);
}

BOOST_AUTO_TEST_CASE(TunRx_test)
{
    // open TUN device
    char dev[IFNAMSIZ] = TUN_DEV;
    int flags = IFF_TUN | IFF_NO_PI;
    int fd = allocate(dev, flags);
    BOOST_CHECK(fd >= 0);

    fd_set socketSet;
    struct timeval selectTimeout;
    char buffer[MAX_BUF_SIZE];
    int nread;

    // reset socket set and add tap descriptor
    FD_ZERO(&socketSet);
    FD_SET(fd, &socketSet);

    // initialize timeout
    selectTimeout.tv_sec = 2;
    selectTimeout.tv_usec = 0;

    // suspend thread until we receive a packet or timeout
    if (select(fd + 1, &socketSet, NULL, NULL, &selectTimeout) == 0) {

        printf("Timeout while reading from interface.\n");
    } else {
        if (FD_ISSET(fd, &socketSet)) {
            if ((nread = read(fd, buffer, MAX_BUF_SIZE)) < 0) {
                printf("Reading from interface failed.\n");
            }
            printf("Read %d bytes from device %s\n", nread, dev);
        }
    }
    close(fd);
}

BOOST_AUTO_TEST_CASE(TunTx_test)
{
    FlowProperties props;
    props.set_addressing_mode(IMPLICIT);

    Gdtp tx_prot;
    tx_prot.set_default_source_address(1); // 16777226 is registered as when creating flow using implicit addressing
    BOOST_CHECK_NO_THROW(tx_prot.initialize());

    Gdtp rx_prot;
    rx_prot.set_default_source_address(33554442); // this is the integer converted representation of 10.0.0.1
    BOOST_CHECK_NO_THROW(rx_prot.initialize());

    // create flow with default requirements
    FlowId id;
    BOOST_CHECK_NO_THROW(id = tx_prot.allocate_flow(DEFAULT_ID, props));

    // feed ping request to GDTP instance
    std::shared_ptr<Data> sdu = make_shared<Data>(ping_request, ping_request + sizeof(ping_request) / sizeof(ping_request[0]));
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
}

BOOST_AUTO_TEST_SUITE_END()
