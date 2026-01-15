#!/bin/bash

BIN=/home/jbyoon/memtis/memtis-userspace/bench_dir/XSBench/openmp-threading
BENCH_RUN="${BIN}/XSBench -t 20 -g 15000 -p 10000000"
BENCH_DRAM=""

if [[ "x${NVM_RATIO}" == "xstatic" ]]; then
    BENCH_DRAM=${STATIC_DRAM}
elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="2048MB"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="4096MB"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="6000MB"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="21800MB"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="32768MB"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="75000MB"
else
    BENCH_DRAM="1GB"
fi


export BENCH_RUN
export BENCH_DRAM
