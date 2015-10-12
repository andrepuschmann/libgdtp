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

/*! \brief Main header file of libgdtp.
 *
 * This header defines the external interface for accessing the functionality
 * of libgdtp.
 */

#ifndef LIBGDTP_H
#define LIBGDTP_H

#include <memory>
#include <vector>
#include <string>
#include "flow_property.h"

namespace libgdtp
{

#define MAX_NUM_FLOWS 255
#define DEFAULT_BELOW_PORT_ID 0

///< datatype definitions
typedef std::vector<uint8_t> Data; ///< Data type used for service data unit payload (from above and below)
typedef uint32_t FlowId;          ///< Flow identifier
typedef uint32_t PortId;          ///< Identifier used for lower and upper layer ports
typedef uint64_t Addr;            ///< Address identifier
typedef uint64_t SeqNo;           ///< Sequence number type

///< Statistics for data handled by an ARQ
typedef struct
{
    uint32_t sdus_from_above;
    uint32_t sdus_for_above;
    uint32_t pdus_from_below;
    uint32_t pdus_for_below;
    uint32_t rtx_pdus;
    uint32_t lost_pdus;
    uint32_t bytes_from_above;
    uint32_t bytes_for_above;
    float fer;
} ArqStats;

///< Overall statistics for all frames that leave or arrive at a node, including broadcast transmissions
typedef struct
{
    uint32_t bytes_from_above;
    uint32_t bytes_from_below;
    uint32_t bytes_for_above;
    uint32_t bytes_for_below;
    uint32_t frames_for_below;
    uint32_t frames_from_below;
} EncoderStats;

///< Both stats combined
typedef struct
{
    ArqStats arq;
    EncoderStats encoder;
} FlowStats;

static const FlowProperties default_props;

/**
 * Public interface for libgdtp
 */
class Gdtp
{
public:
    Gdtp();
    ~Gdtp(); ///< DTOR required

    Gdtp(Gdtp&& rhs); ///< move constructor
    Gdtp& operator=(Gdtp&& rhs); ///< move assignment

    /**
     * \brief Initialize libgdtp.
     *
     * This function needs to be called prior to actually using the lib in order to make sure
     * everything is constructed properly.
     *
     */
    void initialize();

    /**
     * \brief Set the scheduler to be used by the lower layer interface.
     *
     * This function configures the type of scheduler to be used by the lower
     * layer interface.
     *
     * @param type The actual scheduler
     * @param port The lower layer port of which the scheduler should be set
     *
     */
    void set_scheduler_type(const std::string type, const PortId port = DEFAULT_BELOW_PORT_ID);

    /**
     * \brief Set the default source and destination address to be used for flows.
     *
     * @param source The source address
     *
     */
    void set_default_source_address(const Addr source);

    /**
     * \brief Set the default destination and destination address to be used for flows.
     *
     * @param destination The destination address
     *
     */
    void set_default_destination_address(const Addr destination);

    /**
     * @brief Allocate new flow
     * @param id This is an artifical ID that the caller needs to provide.
     * @param props Flow properties
     * @return The actual flow ID
     */
    int allocate_flow(const FlowId id, FlowProperties props = default_props);

    /**
     * @brief Modify the properties of a flow during runtime.
     * @param id The ID of the flow.
     * @param props Flow properties
     */
    void modify_properties(const FlowId id, FlowProperties props);

    /**
     * @brief Obtain transmisstion statistics of a flow
     * @param id The ID of the flow.
     * @return A struct containing current flow statistics.
     */
    FlowStats get_stats(const FlowId id);

    /**
     * @brief Handle data from an upper layer component
     * @param sdu A shared pointer to the data provided.
     * @param id The ID of the flow.
     */
    void handle_data_from_above(std::shared_ptr<Data> data, const FlowId id);

    /**
     * @brief Handle data from an lower layer component
     * @param data A shared pointer to the data provided.
     * @param id The ID of the flow.
     */
    void handle_data_from_below(const PortId id, Data& data);

    /**
     * @brief Query whether there is data to be transmitted or not.
     * @param id The ID of the port.
     * @return True if outgoing queue is not empty.
     */
    bool has_data_for_below(const PortId id);

    /**
     * @brief Retrieve the data to be transmitted on a given port.
     * @param data A reference to a data object to be filled.
     */
    void get_data_for_below(const FlowId id, Data& data);

    /**
     * @brief Inform libgdtp that data has actually been transmitted.
     * @param id The flow ID the data belongs to.
     */
    void set_data_transmitted(const FlowId id);

    /**
     * @brief Query whether there is data available to pass up.
     * @param id The ID of the flow to check.
     * @return True if outgoing queue is not empty.
     */
    bool has_data_for_above(const FlowId id);

    /**
     * @brief Retrieve data received on a flow.
     * @param id The ID of the flow.
     * @return A shared pointer to the actual data.
     */
    std::shared_ptr<Data> get_data_for_above(const FlowId id);

private:
    // non-copyable
    Gdtp(const Gdtp&) = delete;
    Gdtp& operator=(const Gdtp&) = delete;

    // PIMPL idiom
    class GdtpImpl;
    std::unique_ptr<GdtpImpl> impl_;
};

} // end of gdtp

#endif // LIBGDTP_H
