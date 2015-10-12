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

#ifndef SCHEDULER_FACTORY_H
#define SCHEDULER_FACTORY_H

#include "fifo_scheduler.h"
#include "priority_scheduler.h"
#include "implicitack_scheduler.h"

namespace libgdtp
{

enum Scheduler
{
    FIFO,
    PRIORITY,
    IMPLICITACK
};

static std::map<std::string, Scheduler> scheduler_map_ = {{"fifo", FIFO},
                                                          {"priority", PRIORITY},
                                                          {"implicitack", IMPLICITACK}};

class SchedulerFactory
{
    public:
        static std::unique_ptr<SchedulerBase> make_scheduler(const std::string type)
        {
            Scheduler scheduler = scheduler_map_[type];
            switch (scheduler)
            {
                case FIFO:
                    return std::unique_ptr<SchedulerBase>(new FifoScheduler());
                case PRIORITY:
                    return std::unique_ptr<SchedulerBase>(new PriorityScheduler());
                case IMPLICITACK:
                    return std::unique_ptr<SchedulerBase>(new ImplicitAckScheduler());
                default:
                    throw GdtpException("Unknown scheduler type.");
            }
        }
};

} // namespace libgdtp

#endif // SCHEDULER_FACTORY_H
