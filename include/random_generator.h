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

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <stdio.h>
#include <time.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

namespace libgdtp
{

class random_generator
{
private:
    // private constructor
    random_generator() {
        generator.seed(static_cast<unsigned int>(std::time(0)));
    }
    random_generator(const random_generator&) {}  // private copy constructor
    random_generator& operator=(random_generator const&) {} // private assignment operator
    ~random_generator() {}
    static random_generator* instance;
    boost::mt19937 generator;

public:
    static random_generator& get_instance() {
        static random_generator _instance;
        return _instance;
    }

    static void destroy() {
        if (instance)
            delete instance;
        instance = 0;
    }

    uint32_t uniform_0_to_n(uint32_t n) {
        boost::uniform_int<> window(0, n);
        boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(generator, window);
        return die();
    }

    float uniform_0_to_1(void) {
        boost::uniform_real<> window(0,1);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<> > die(generator, window);
        return die();
    }
};

} // end of libgdtp

#endif // RANDOM_GENERATOR_H
