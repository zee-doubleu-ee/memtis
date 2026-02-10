#!/bin/bash

BIN="${USPACE_DIR}/bin"
BENCH_RUN="${BIN}/dbtest --verbose --bench ycsb --num-threads 20 --scale-factor 400000 --ops-per-worker=1000000000 --slow-exit"
BENCH_DRAM=""

#####
# Silo ~59500MB memory footprint
#####

if [[ "x${NVM_RATIO}" == "x1:32" ]]; then
    BENCH_DRAM="1803M"
elif [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="3500M"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="6600M"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="11900M"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="19800M"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="29750M"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="70000M"
fi

export BENCH_RUN
export BENCH_DRAM
