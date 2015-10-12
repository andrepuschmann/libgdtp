# libgdtp [![Build Status](https://travis-ci.org/andrepuschmann/libgdtp.svg?branch=master)](https://travis-ci.org/andrepuschmann/libgdtp)

libgdtp is a generic data transfer protocol implementation that may be used
to build packet-based communication systems for various purposes.
It has been successfully employed for building fixed as well as dynamic spectrum 
access systems. It can be used standalone or paired with a software-defined
radio framework or protocol simulator.
libgdtp supports multiple flows with possibly different communication requirements,
e.g., different levels of reliablity and priorities.
libgdtp relies on Protocol Buffers for data serialization.


### Features:

- Reliable and unreliable transfer service
- Simple stop and wait ARQ
- Multi-flow support
- Scheduler (FIFO, priority-based)
- Wrapper components for Iris and GNU Radio


### Getting started:

1. Install some basic packet dependencies onto your system. E.g., using Ubuntu, invoke the following:

   ```bash
$ sudo apt-get install build-essential libboost-all-dev git cmake libprotobuf-dev protobuf-compiler protobuf-c-compiler liblog4cxx10-dev
```

2. Clone the repo to your machine:

    ```bash
$ git clone https://github.com/andrepuschmann/libgdtp.git libgdtp
```
3. Create a build directory and build.

    ```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make test
$ make install
```
   
   Note: Building libgdtp for debugging goes like this ..

    ```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```
