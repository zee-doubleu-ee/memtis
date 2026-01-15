#!/bin/bash

BENCHMARKS="XSBench"
NVM_RATIO="1:4 1:8 1:16"

sudo dmesg -c

for BENCH in ${BENCHMARKS};
do
    for NR in ${NVM_RATIO};
    do
	./scripts/run_bench.sh -B ${BENCH} -R ${NR} -D 12GB -V test
    done
done
