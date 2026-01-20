#!/bin/bash

BENCHMARKS="XSBench graph500 gapbs-pr liblinear silo btree"

USPACE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CPU_NODE0=$(cat /sys/devices/system/node/node0/cpulist)
export USPACE_DIR

sudo dmesg -c

# enable THP
echo "always" | sudo tee /sys/kernel/mm/transparent_hugepage/enabled > /dev/null
echo "always" | sudo tee /sys/kernel/mm/transparent_hugepage/defrag > /dev/null
# decrease uncore freq
./scripts/set_uncore_freq.sh on

for BENCH in ${BENCHMARKS};
do
    export GOMP_CPU_AFFINITY=$CPU_NODE0
    if [[ -e ./bench_cmds/${BENCH}.sh ]]; then
	source ./bench_cmds/${BENCH}.sh
    else
	echo "ERROR: ${BENCH}.sh does not exist."
	continue
    fi

    mkdir -p results/${BENCH}/all-cxl/static
    LOG_DIR=results/${BENCH}/all-cxl/static

    free;sync;echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null;free;

    if [[ "x${BENCH}" =~ "xspeccpu" ]]; then
	/usr/bin/time -f "execution time %e (s)" \
	    taskset -c $CPU_NODE0 numactl -m 1 ${BENCH_RUN} < ${BENCH_ARG} 2>&1 \
	    | tee ${LOG_DIR}/output.log
    else
	/usr/bin/time -f "execution time %e (s)" \
	    numactl -N 0 -m 1 ${BENCH_RUN} 2>&1 \
	    | tee ${LOG_DIR}/output.log
    fi
done

./scripts/set_uncore_freq.sh off
