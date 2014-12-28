#!/bin/sh

start=$1

for i in `seq 10`
do
    day=`date -d "$start -$i days" +%Y%m%d`
    python delete_error_monitor.py -d $day
    echo "delete $day ok!"
done
