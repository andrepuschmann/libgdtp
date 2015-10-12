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

/*! \brief Logger implementation for libgdtp.
 *
 * This header defines the logger functionality for libgdtp.
 * At the moment, log4cxx is used as a logging library, which may be replaced
 * by another library or an alternate implementation in the future.
 * The current implementation is loosly based on the GNU Radio logger class.
 */

#ifndef LOGGER_H
#define LOGGER_H

#define ENABLE_LOGGER ///< Undef to disable logger
#undef HAVE_LOG4CXX

#ifdef HAVE_LOG4CXX
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/logmanager.h"
#include "log4cxx/simplelayout.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace libgdtp
{

#ifdef ENABLE_LOGGER

#ifdef HAVE_LOG4CXX

#define DECLARE_LOGPTR(logger) static GdtpLogger logger;
#define ASSIGN_LOGPTR(logger, name) GdtpLogger logger = GdtpLogger(name);
#define CONFIG_LOGGER(config) logger_.initialize(config);
#define DESTROY_LOGGER() log4cxx::LogManager::shutdown();

class GdtpLogger{
public:
    GdtpLogger(const std::string name) :
        logger_(log4cxx::LoggerPtr(log4cxx::Logger::getLogger(name)))
    {}

    void initialize(const std::string config = "log4cxx.conf")
    {
        // check if config file exists
        std::string config_file(config.c_str());
        struct stat buffer;
        if (stat(config_file.c_str(), &buffer) == 0) {
            log4cxx::PropertyConfigurator::configure(config_file);
        } else {
            log4cxx::PatternLayout *layout = new log4cxx::PatternLayout("%d %-5p %m%n");
            log4cxx::ConsoleAppender* console_appender = new log4cxx::ConsoleAppender(layout, "logfile");
            log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();
            log4cxx::BasicConfigurator::configure(console_appender);
        }
    }

    log4cxx::LoggerPtr get4cxx(void)
    {
        return this->logger_;
    }

private:
    log4cxx::LoggerPtr logger_;
};

#define LOG_DEBUG(message) LOG4CXX_DEBUG(logger_.get4cxx(), message)
#define LOG_INFO(message) LOG4CXX_INFO(logger_.get4cxx(), message)
#define LOG_WARNING(message) LOG4CXX_INFO(logger_.get4cxx(), message)
#define LOG_ERROR(message) LOG4CXX_ERROR(logger_.get4cxx(), message)


#else // HAVE_LOG4CXX

// logging to std
#define DECLARE_LOGPTR(logger)
#define ASSIGN_LOGPTR(logger, name)
#define CONFIG_LOGGER(config)
#define DESTROY_LOGGER()

#define LOG_DEBUG(msg) std::cout << "DEBUG: " << msg << std::endl
#define LOG_INFO(msg) std::cout << "INFO: " << msg << std::endl
#define LOG_ERROR(msg) std::cout << "ERROR: " << msg << std::endl

#endif // HAVE_LOG4CXX

#else // ENABLE_LOGGER

// turn off logging
#define DECLARE_LOGPTR(logger)
#define ASSIGN_LOGPTR(logger,name)
#define CONFIG_LOGGER(config)
#define DESTROY_LOGGER()

#define LOG_DEBUG(message)
#define LOG_INFO(message)
#define LOG_ERROR(message)

#endif // ENABLE_LOGGER

} // namespace libgdtp

#endif // LOGGER_H
