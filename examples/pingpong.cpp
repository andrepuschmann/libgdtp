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

#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/thread.hpp"
#include "boost/format.hpp"
#include <boost/program_options.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include "libgdtp.h"
#include "channel.h"
#define HAVE_TIME
#include "logger.h"

namespace po = boost::program_options;
using namespace libgdtp;

#define UPPER_PORT_RANGE_START 1
#define BELOW_PORT_ID 0
#define TIMEOUT 20

void tx_app_handler(Gdtp& prot, Data& tx_buffer, const FlowId id, const uint32_t num_tx_frames, const uint32_t frame_size)
{
    for (int i = 1; i <= num_tx_frames; i++) {
        // create dummy upper layer SDU and feed it to GDTP instance
        uint8_t pattern = random_generator::get_instance().uniform_0_to_n(255);
        std::shared_ptr<Data> data = std::make_shared<Data>(frame_size, pattern);
        prot.handle_data_from_above(data, id);

        // append to tx_buffer for later comparision
        tx_buffer.insert(tx_buffer.end(), data->begin(), data->end());
    }
}

void rx_app_handler(Gdtp& prot, Data& rx_buffer, const FlowId id)
{
    try
    {
        while (true) {
            std::shared_ptr<Data> data = prot.get_data_for_above(id);
            rx_buffer.insert(rx_buffer.end(), data->begin(), data->end());
        }
    }
    catch(boost::thread_interrupted)
    {
        std::cout << "Rx application thread " << boost::this_thread::get_id() << " was interrupted." << std::endl;
    }
}

void tx_handler(Gdtp& prot, Channel &channel, const std::string name, const PortId id)
{
    try
    {
        Data frame;
        while (true) {
            // this call may block if no frames are present
            prot.get_data_for_below(id, frame);
            channel.pushBack(frame); // pass frame down to lower layer
            prot.set_data_transmitted(id);
        }
    }
    catch(boost::thread_interrupted)
    {
        std::cout << "Tx thread " << boost::this_thread::get_id() << " was interrupted." << std::endl;
    }
}

void rx_handler(Gdtp& prot, Channel &channel, const std::string name, const PortId id)
{
    try
    {
        while (true) {
            Data frame;
            channel.popFront(frame);
            prot.handle_data_from_below(id, frame);
        }
    }
    catch(boost::thread_interrupted)
    {
        std::cout << "Rx thread " << boost::this_thread::get_id() << " was interrupted." << std::endl;
    }
}

int main(int argc, char *argv[])
{
    bool print_header;
    int num_tx;
    float fer1;
    float fer2;
    bool reliable;
    uint32_t num_tx_frames;
    uint32_t sdu_size;
    Addr node1_addr;
    Addr node2_addr;
    uint64_t max_seq_no;
    uint32_t max_num_rtx;
    std::string scheduler;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "help message")
            ("header", po::value<bool>(&print_header)->default_value(false), "Whether to print header before results")
            ("num_tx", po::value<int>(&num_tx)->default_value(1), "Number of transmitters")
            ("fer1", po::value<float>(&fer1)->default_value(float(0.0)), "FER of transmitter to receiver channel")
            ("fer2", po::value<float>(&fer2)->default_value(float(0.0)), "FER of receiver to transmitter channel")
            ("reliable", po::value<bool>(&reliable)->default_value(true), "Whether to use reliable transfer mode")
            ("num_frames", po::value<uint32_t>(&num_tx_frames)->default_value(10), "number of frames to transmit")
            ("sdu_size", po::value<uint32_t>(&sdu_size)->default_value(1000), "SDU size")
            ("node1_addr", po::value<Addr>(&node1_addr)->default_value(1), "address of first node")
            ("node2_addr", po::value<Addr>(&node2_addr)->default_value(2), "address of second node")
            ("max_seq_no", po::value<uint64_t>(&max_seq_no)->default_value(127), "maximum sequence number")
            ("max_num_rtx", po::value<uint32_t>(&max_num_rtx)->default_value(3), "maximum number of retransmissions")
            ("scheduler", po::value<std::string>(&scheduler)->default_value("fifo"), "scheduler")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << boost::format("Simple GDTP loop example %s") % desc << std::endl;
        return ~0;
    }

    // check for bogus parameter settings
    if (num_tx < 1 || num_tx > 2) {
        std::cout << boost::format("Number of transmitters must be between one and two.") << std::endl;
        return ~0;
    }

    // define default flow properties
    FlowProperties props(reliable ? RELIABLE : UNRELIABLE,
                       99,
                       EXPLICIT,
                       max_seq_no,
                       TIMEOUT,
                       max_num_rtx);

    // creating protocol instances
    Gdtp node1_prot;
    node1_prot.set_default_source_address(node1_addr);
    node1_prot.set_default_destination_address(node2_addr);
    node1_prot.set_scheduler_type(scheduler);
    node1_prot.initialize();

    Gdtp node2_prot;
    node2_prot.set_default_source_address(node2_addr);
    node2_prot.set_default_destination_address(node1_addr);
    node2_prot.set_scheduler_type(scheduler);
    node2_prot.initialize();

    // create a thread-safe buffer that both lower layer handler can use to exchange data
    Channel channel_n1_to_n2_(num_tx_frames * max_num_rtx, fer1), channel_n2_to_n1_(num_tx_frames * max_num_rtx, fer2);

    // create Tx and Rx buffer
    std::map< int, Data > tx_buffer, rx_buffer;

    // spawn upper layer application on transmitting node
    boost::ptr_vector<boost::thread> tx_app_threads, threads_;
    std::vector<FlowId> tx_ids;

    // there will always be at least one transmitter
    FlowId id = node1_prot.allocate_flow(UPPER_PORT_RANGE_START, props);

    // register same flow at node2 to fix properties, waiting prevents to create same random flow-id
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    node2_prot.allocate_flow(UPPER_PORT_RANGE_START, props);

    tx_app_threads.push_back(new boost::thread(boost::bind(tx_app_handler, boost::ref(node1_prot), boost::ref(tx_buffer[id]), id, num_tx_frames, sdu_size)));
    threads_.push_back(new boost::thread(boost::bind(rx_app_handler, boost::ref(node2_prot), boost::ref(rx_buffer[id]), UPPER_PORT_RANGE_START)));
    tx_ids.push_back(id);

    // start second trasnmitter on node2
    if (num_tx > 1) {
        id = node2_prot.allocate_flow(UPPER_PORT_RANGE_START + 1, props);
        node1_prot.allocate_flow(UPPER_PORT_RANGE_START + 1, props);
        tx_app_threads.push_back(new boost::thread(boost::bind(tx_app_handler, boost::ref(node2_prot), boost::ref(tx_buffer[id]), id, num_tx_frames, sdu_size)));
        threads_.push_back(new boost::thread(boost::bind(rx_app_handler, boost::ref(node1_prot), boost::ref(rx_buffer[id]), UPPER_PORT_RANGE_START + 1)));
        tx_ids.push_back(id);
    }

    // create tx and rx threads for each node
    threads_.push_back(new boost::thread(boost::bind(rx_handler, boost::ref(node1_prot), boost::ref(channel_n2_to_n1_), "node1", BELOW_PORT_ID)));
    threads_.push_back(new boost::thread(boost::bind(tx_handler, boost::ref(node1_prot), boost::ref(channel_n1_to_n2_), "node1", BELOW_PORT_ID)));

    threads_.push_back(new boost::thread(boost::bind(rx_handler, boost::ref(node2_prot), boost::ref(channel_n1_to_n2_), "node2", BELOW_PORT_ID)));
    threads_.push_back(new boost::thread(boost::bind(tx_handler, boost::ref(node2_prot), boost::ref(channel_n2_to_n1_), "node2", BELOW_PORT_ID)));

    // wait for tx app handler
    boost::ptr_vector<boost::thread>::iterator it;
    for (it = tx_app_threads.begin(); it != tx_app_threads.end(); ++it) {
        it->join();
    }

    // ... and finally, interrupt remaining threads
    boost::this_thread::sleep(boost::posix_time::seconds(3));
    for (it = threads_.begin(); it != threads_.end(); ++it) {
        it->interrupt();
        it->join();
    }

    // check if all is good
    bool is_equal = std::equal(tx_buffer.begin(), tx_buffer.end(), rx_buffer.begin());
    if (not is_equal) {
        std::cout << "Error during transmission!" << std::endl;
        return ~1;
    }

    // calculate effiency, at first, assume only node1 transmits data
    uint32_t bytes_to_tx = 0, bytes_node1_sent = 0, bytes_node2_sent = 0, num_frames = 0;
    FlowStats node1_stats = node1_prot.get_stats(tx_ids[0]);
    FlowStats node2_stats = node2_prot.get_stats(UPPER_PORT_RANGE_START);
    bytes_to_tx += node1_stats.arq.bytes_from_above;
    bytes_node1_sent += node1_stats.encoder.bytes_for_below;
    bytes_node2_sent += node2_stats.encoder.bytes_for_below;
    num_frames += node1_stats.encoder.frames_for_below + node2_stats.encoder.frames_for_below;

    // only add stats for outgoing flow of second node
    if (num_tx > 1) {
        node2_stats = node2_prot.get_stats(tx_ids[1]);
        bytes_to_tx += node2_stats.arq.bytes_from_above;
    }

    // get efficiency
    float eff = bytes_to_tx / float(bytes_node1_sent + bytes_node2_sent);

    // print results, seperated by tabs
    if (print_header)
        std::cout << boost::format("size\tfer1\tfer2\teff\tframes") << std::endl;

    std::cout << boost::format("%d\t%.2f\t%.2f\t%.2f\t%d") % sdu_size % fer1 % fer2 % eff % num_frames<< std::endl;
    return 0;
}

