#!/bin/bash
ulimit -s unlimited
/home/jbyoon/memtis/memtis-userspace/bench_dir/gups/gups 8 200000000 30 8 0 0 0 2>> ${cmd_DIR}/gups_error_${cmd_CONF}.err | tee -a ${cmd_DIR}/gups_output_${cmd_CONF}.out > /dev/tty