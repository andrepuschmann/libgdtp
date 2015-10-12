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

#define BOOST_TEST_MODULE Misc_test

#include <boost/test/unit_test.hpp>
#include "libgdtp.h"

using namespace std;
using namespace libgdtp;

using namespace boost;
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Misc_test)


BOOST_AUTO_TEST_CASE(Empty_test)
{
    BOOST_CHECK(1 == 1);
}

BOOST_AUTO_TEST_CASE(Init_test)
{
    Gdtp prot;
}

BOOST_AUTO_TEST_SUITE_END()
