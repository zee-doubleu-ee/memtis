#!/bin/bash

BENCHMARKS="XSBench gapbs-pr liblinear silo btree graph500"
#BENCHMARKS="graph500"
NVM_RATIO="1:16 1:8 1:2 1:0"

sudo dmesg -c

for BENCH in ${BENCHMARKS};
do
    for NR in ${NVM_RATIO};
    do
	./scripts/run_bench.sh -B ${BENCH} -R ${NR} -V test --cxl
    done
done
