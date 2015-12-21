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

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <stdexcept>

namespace libgdtp
{

/** The base exception class for libgdtp
*
*   All other exceptions within the library inherit from this base class
*/
class GdtpException
        : public std::exception
{
public:
    GdtpException(const std::string &message) throw()
        :exception(), message_(message)
    {};
    virtual const char* what() const throw()
    {
        return message_.c_str();
    };
    virtual ~GdtpException() throw()
    {};

private:
    std::string message_; ///< Message describing the cause of the exception.
};

/// Exception thrown when parameter value is wrong
class ParameterException : public GdtpException
{
public:
    ParameterException(const std::string &message) throw()
        :GdtpException(message)
    {};
};

/// Exception thrown when frame couldn't be decoded
class DecodeException : public GdtpException
{
public:
    DecodeException(const std::string &message) throw()
        :GdtpException(message)
    {};
};

} // libgdtp namespace

#endif /* EXCEPTIONS_H_ */
