#!/bin/bash

GENERAL_CPUS="0-1"
TESTING_CPUS="2-3"
MEMS=0

# restrict all* processes from using the testing cpus
# *except for kernel processes and the like
echo "restricting usage of testing cpus"
for dir in /sys/fs/cgroup/*/; do
    if [ -d $dir ] && [ "$dir" != "/sys/fs/cgroup/" ]; then
        echo "Setting cpuset.cpus=$GENERAL_CPUS for $dir"
        echo $GENERAL_CPUS > ${dir}cpuset.cpus
    fi
done

echo "Creating cgroups"
# setup cgroup for testing
mkdir /sys/fs/cgroup/testing
echo $MEMS >         /sys/fs/cgroup/testing/cpuset.mems
echo $TESTING_CPUS > /sys/fs/cgroup/testing/cpuset.cpus
echo 1 >             /sys/fs/cgroup/testing/cpuset.cpus.exclusive

# setup cgroup for other processes
mkdir /sys/fs/cgroup/other
echo $MEMS >         /sys/fs/cgroup/other/cpuset.mems
echo $GENERAL_CPUS > /sys/fs/cgroup/other/cpuset.cpus

# move all possible processes from root cgroup to other cgroup
echo "Attempting to move all processes from root cgroup to other cgroup"
while IFS= read -r line; do
    if echo $line > $1cgroup.procs 2> /dev/null; then
        echo "Moved process $line to cgroup 'other'"
    else
        echo "Couldn't move process $line to cgroup"
    fi
done < /sys/fs/cgroup/cgroup.procs
