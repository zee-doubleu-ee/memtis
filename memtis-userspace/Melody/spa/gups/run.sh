#!/bin/bash
RUNDIR=$(echo "$(dirname "$PWD")")
GUPS_RUN_DIR="${RUNDIR}/gups"
RSTDIR="${GUPS_RUN_DIR}/rst"
#PERF="${RUNDIR}/linux/tools/perf/perf"
PERF="perf"

if [[ $# != 1 && $# != 2 ]]; then
  echo ""
  echo "$0 wi.txt"
  echo "$0 w.txt 1"
  echo ""
  exit
fi

WF=$1
WID=$2
if [[ $# == 1 ]]; then
  warr=($(cat $WF | awk '{print $1}'))
  marr=($(cat $WF | awk '{print $2}'))
elif [[ $# == 2 ]]; then
  warr=($(cat $WF | awk -vline=$WID 'NR == line {print $1}'))
  marr=($(cat $WF | awk -vline=$WID 'NR == line {print $2}'))
fi

echo "==> Result directory: $RSTDIR"

source $RUNDIR/config.sh || exit
echo "Checking perf ..."
#[[ -e $PERF ]] || exit
type $PERF >/dev/null 2>&1 || exit
echo "Finished checking"

TIME_FORMAT="\n\n\nReal: %e %E\nUser: %U\nSys: %S\nCmdline: %C\nAvg-total-Mem-kb: %K\nMax-RSS-kb: %M\nSys-pgsize-kb: %Z\nNr-voluntary-context-switches: %w\nCmd-exit-status: %x"
if [[ ! -e /usr/bin/time ]]; then
  echo "Please install GNU time first!"
  exit
fi

run_one_exp()
{
  local w=$1
  local et=$2
  local id=$3
  local mem=$4
  local run_cmd="bash cmd.sh"
  flush_fs_caches

  echo "    => Running [$w - $et - $id], date:$(date) ..."
  if [[ $et == "L100" ]]; then
    run_cmd="numactl --cpunodebind 0 --membind 0 -- ""${run_cmd}"
  elif [[ $et == "L0" ]]; then
    run_cmd="numactl --cpunodebind 0 --membind 1 -- ""${run_cmd}"
  else
    run_cmd="numactl --cpunodebind 0 -- ${run_cmd}"
  fi

  local output_dir="$RSTDIR"
  [[ ! -d ${output_dir} ]] && mkdir -p ${output_dir}

  local logf=${output_dir}/${et}-${id}.log
  local timef=${output_dir}/${et}-${id}.time
  local output=${output_dir}/${et}-${id}.output
  local memf=${output_dir}/${et}-${id}.mem
  local sysinfof=${output_dir}/${et}-${id}.sysinfo
  local perfoutput=${output_dir}/${et}-${id}.data

  local perf_events="instructions,cycles,dTLB-load-misses"
  perf_events="${perf_events}"",CYCLE_ACTIVITY.STALLS_MEM_ANY,EXE_ACTIVITY.BOUND_ON_STORES"
  perf_events="${perf_events}"",CYCLE_ACTIVITY.STALLS_L1D_MISS,CYCLE_ACTIVITY.STALLS_L2_MISS,CYCLE_ACTIVITY.STALLS_L3_MISS"
  perf_events="${perf_events}"",EXE_ACTIVITY.1_PORTS_UTIL,EXE_ACTIVITY.2_PORTS_UTIL,PARTIAL_RAT_STALLS.SCOREBOARD"
  run_cmd="sudo -E $PERF stat -e ${perf_events} -o $perfoutput  ""${run_cmd}"
  # -E 옵션을 추가해서 환경 변수를 sudo 실행환경까지 유지하도록 함  

  {
    echo "$run_cmd" | tee r.sh
    echo "Start: $(date)"
    get_sysinfo > $sysinfof 2>&1
    /usr/bin/time -f "${TIME_FORMAT}" --append -o ${timef} bash r.sh > $output 2>&1 &
    cpid=$!
    monitor_resource_util >>$memf 2>&1 &
    mpid=$!
    disown $mpid # avoid the "killed" message
    wait $cpid 2>/dev/null
    kill -9 $mpid >/dev/null 2>&1
    echo "End: $(date)"
    echo "" && echo "" && echo "" && echo ""
    cat r.sh
    echo ""
    cat cmd.sh
    rm -rf r.sh
    sleep 5
  } >> $logf
}

run_seq()
{
  local type=$1
  local id=$2
  check_cxl_conf
  for ((i = 0; i < ${#warr[@]}; i++)); do
    w=${warr[$i]}
    m=${marr[$i]}
    cd "$w"
    run_one_exp "$w" "$type" "$id" "$m"
    cd ../
  done
  return
}

main()
{
  local base_dir="${RSTDIR}/gups_task"
  local n=1
  while [[ -d "${base_dir}_$n" ]]; do
    ((n++))
  done
  RSTDIR="${base_dir}_$n"
  mkdir -p "$RSTDIR"
  echo "==> Results will be saved to : $RSTDIR"

  # -------- export해서 cmd.sh에서 사용할수있게함 ------
  export cmd_DIR=$RSTDIR
  export cmd_CONF=$n

  echo "Run LOCAL ..."
  run_seq "L100" "100"
  echo "Run REMOTE ..."
  run_seq "L0" "1"
}

main
echo "FINISHED"
exit
