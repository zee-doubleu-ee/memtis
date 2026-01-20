#!/bin/bash

COMMAND=$1
CGROUP_NAME=$2
CGROUP_DIR=/sys/fs/cgroup
BASH_PID=$3

print_help() {
     echo "$0 [ remove <cgroup name> | add <cgroup name> <bash pid> [enable | disable]]"
}

if [ "x$1" == "xremove" ]; then
	sudo rmdir ${CGROUP_DIR}/${CGROUP_NAME}
        exit
elif [ "x$1" != "xadd" ]; then
	echo "$0 Invalid command..."
	print_help
	exit
fi

if [ ! -d "${CGROUP_DIR}/${CGROUP_NAME}" ]; then
	sudo mkdir -p ${CGROUP_DIR}/${CGROUP_NAME}
fi

echo "+memory" | sudo tee ${CGROUP_DIR}/cgroup.subtree_control > /dev/null
echo "+cpuset" | sudo tee ${CGROUP_DIR}/cgroup.subtree_control > /dev/null

echo ${BASH_PID} | sudo tee ${CGROUP_DIR}/${CGROUP_NAME}/cgroup.procs
if [ "x$4" == "xenable" ]; then
    echo "enabled" | sudo tee ${CGROUP_DIR}/${CGROUP_NAME}/memory.htmm_enabled > /dev/null
    exit
elif [ "x$4" == "xdisable" ]; then
    echo "disabled" | sudo tee ${CGROUP_DIR}/${CGROUP_NAME}/memory.htmm_enabled > /dev/null
    exit
fi
print_help
