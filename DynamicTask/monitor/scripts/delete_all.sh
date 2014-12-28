#!/bin/sh

start=$1

for i in `seq 30`
do
    day=`date -d "$start -$i days" +%Y%m%d`
    python delete_task_monitor.py $day
    echo "delete $day ok!"
done
