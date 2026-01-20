#!/bin/bash

sudo modprobe intel-uncore-frequency

if [[ "x$1" == "xon" ]]; then
    echo 800000 | sudo tee /sys/devices/system/cpu/intel_uncore_frequency/package_01_die_00/max_freq_khz > /dev/null
    echo 800000 | sudo tee /sys/devices/system/cpu/intel_uncore_frequency/package_01_die_00/min_freq_khz > /dev/null
elif [[ "x$1" == "xoff" ]]; then
    # default values
    echo 2400000 | sudo tee /sys/devices/system/cpu/intel_uncore_frequency/package_01_die_00/max_freq_khz > /dev/null
    echo 1200000 | sudo tee /sys/devices/system/cpu/intel_uncore_frequency/package_01_die_00/min_freq_khz > /dev/null
else
    echo "usage: ./set_uncore_freq.sh [on/off]"
fi
