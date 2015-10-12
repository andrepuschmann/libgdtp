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

#ifndef CHANNEL_H
#define CHANNEL_H

/**
 * @brief The channel class
 *
 * This is a simple helper class to implement/emulate a wireless
 * communication channel with a certain error rate.
 */

#include "buffer.h"
#include "random_generator.h"
#include "libgdtp.h"

namespace libgdtp
{

class Channel
{
public:
    Channel(const size_t capacity = 1, const float fer = 0) :
        buffer_(capacity),
        fer_(fer)
    {
        assert(fer >= 0 && fer < 1);
    }

    void pushBack(const Data& sdu)
    {
        float rand = random_generator::get_instance().uniform_0_to_1();
        if (fer_ <= rand)
            buffer_.pushBack(sdu);
    }

    void popFront(Data& sdu)
    {
        buffer_.popFront(sdu);
    }

private:
    Buffer<Data> buffer_;
    float fer_;
};

} // namespace gdtp

#endif // CHANNEL_H
