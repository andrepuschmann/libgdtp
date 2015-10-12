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

/**
 * A thread-safe class that implements a fixed-size buffer.
 * Inspired by Iris' MessageQueue.h.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/call_traits.hpp>
#include <queue>

template<class T>
class Buffer : boost::noncopyable
{
public:
    typedef std::queue<T> container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::value_type value_type;
    typedef typename boost::call_traits<value_type>::param_type param_type;

    explicit Buffer(size_type capacity) : capacity_(capacity) {}
    explicit Buffer() : capacity_(10) {}

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void pushBack(T const& data)
    {
        boost::mutex::scoped_lock lock(mutex_);
        while (container_.size() >= capacity_) {
           notFullCond_.wait(lock);
        }
        container_.push(data);
        lock.unlock();
        notEmptyCond_.notify_one();
    }

    bool tryPop(T& value)
    {
        boost::mutex::scoped_lock lock(mutex_);
        if (container_.empty()) {
            return false;
        }
        value = container_.front();
        container_.pop();
        lock.unlock();
        notFullCond_.notify_one();
        return true;
    }

    void popFront(T& value)
    {
        boost::mutex::scoped_lock lock(mutex_);
        while (container_.empty()) {
            notEmptyCond_.wait(lock);
        }
        value = container_.front();
        container_.pop();
        lock.unlock();
        notFullCond_.notify_one();
    }

    size_type size() {
        boost::mutex::scoped_lock lock(mutex_);
        return container_.size();
    }

    size_type capacity() {
        boost::mutex::scoped_lock lock(mutex_);
        return capacity_;
    }

    bool isEmpty() {
        boost::mutex::scoped_lock lock(mutex_);
        return container_.empty();
    }

    bool isNotEmpty() const
    {
        boost::mutex::scoped_lock lock(mutex_);
        return !container_.empty();
    }

private:
    container_type container_;
    size_type capacity_;
    mutable boost::mutex mutex_;
    boost::condition_variable notEmptyCond_;
    boost::condition_variable notFullCond_;
};


#endif // BUFFER_H
