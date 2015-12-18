#!/bin/bash
# Run the program for the huge data file
LD_PRELOAD="/home/nathaniel/.tgd/libs/libNvidia_gfx_debugger.so" build/Project2 data/imgn41w122_13.raw 10812 10812 8 8 10800 10800 41.0 -122.0 40.419100 -121.532891 data/hugepath.polyline

