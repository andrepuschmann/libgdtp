#!/usr/bin/env python
"""
Benchmarking GDTP effiency with varying channel conditions and
frame sizes.
"""

import subprocess
import sys

#frame_size_interval = range(10, 100, 10)
frame_size_interval = range(100, 1600, 100)
#frame_size_interval = range(2000, 11000, 1000)
num_frames = 1000

fer_interval = [x * 0.1 for x in range(0, 5)]
#print fer_interval
#fer1 = 0.2
#fer2 = 0.3

for fer in fer_interval:
    for frame_size in frame_size_interval:
        # run random algorithm, increase no of channels, two users
        cmd = ["../build/examples/simple_loop",
                                "--num_frames=%d" % num_frames,
                                "--frame_size=%d" % frame_size,
                                "--fer1=%f" % fer,
                                "--fer2=%f" % fer ]
        print cmd
        ret = subprocess.call(cmd)
        if ret != 0:
            print "Error occured!"
            sys.exit(-1)

    print ""
