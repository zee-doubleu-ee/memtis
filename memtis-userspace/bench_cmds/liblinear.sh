#!/bin/bash
BENCH_BIN="${USPACE_DIR}/bench_dir/liblinear-multicore-2.50"

# anon footprint 79640MB
# file footprint 21581MB

BENCH_RUN="${BENCH_BIN}/train -s 6 -m 20 ${BENCH_BIN}/datasets/kdd12"
# Liblinear requires a dataset file (kdd12)
# Please refer to memtis-userspace/bench_dir/README.md for downloading this dataset

if [[ "x${NVM_RATIO}" == "x1:16" ]]; then
    BENCH_DRAM="4150M"
elif [[ "x${NVM_RATIO}" == "x1:8" ]]; then
    BENCH_DRAM="8000M"
elif [[ "x${NVM_RATIO}" == "x1:4" ]]; then
    BENCH_DRAM="14128M"
elif [[ "x${NVM_RATIO}" == "x1:2" ]]; then
    BENCH_DRAM="23000M"
elif [[ "x${NVM_RATIO}" == "x1:1" ]]; then
    BENCH_DRAM="35320M"
elif [[ "x${NVM_RATIO}" == "x1:0" ]]; then
    BENCH_DRAM="80000M"
fi


export BENCH_RUN
export BENCH_DRAM
