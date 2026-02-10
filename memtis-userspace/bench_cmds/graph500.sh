#!/bin/bash

BIN="${USPACE_DIR}/bench_dir/graph500/src"
BENCH_RUN="${BIN}/graph500_reference_bfs_sssp 27 15"
BENCH_DRAM=""


if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="3850M"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="7200M"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="13107M"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="21800M"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="32768M"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="75000M"
fi


export BENCH_RUN
export BENCH_DRAM
