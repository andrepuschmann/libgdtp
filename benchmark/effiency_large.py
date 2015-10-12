#!/usr/bin/env python
"""
Benchmarking GDTP effiency with varying channel conditions and
frame sizes.
"""

import subprocess
import sys



frame_size_interval = range(10, 100, 10)
frame_size_interval += range(100, 1600, 100)
frame_size_interval += range(2000, 11000, 1000)
num_frames = 10000

#./examples/simple_loop --fer1=0.0 --fer2=0.0 --num_frames=10000 --frame_size=200 --tx_addr=17000000000000000000 --rx_addr=18000000000000000000 --max_seq_no=18000000000000000000

fer_interval = [x * 0.1 for x in range(0, 5)]
fer_interval = [0.4]

for fer in fer_interval:
    for frame_size in frame_size_interval:
        # run random algorithm, increase no of channels, two users
        ret = subprocess.call(["../build/examples/simple_loop",
                                "--num_frames=%d" % num_frames,
                                "--frame_size=%d" % frame_size,
                                "--tx_addr=17000000000000000000",
                                "--rx_addr=18000000000000000000",
                                "--max_seq_no=18000000000000000000",
                                "--fer1=%f" % fer,
                                "--fer2=%f" % fer ])
        if ret != 0:
            print "Error occured!"
            sys.exit(-1)

    print ""
