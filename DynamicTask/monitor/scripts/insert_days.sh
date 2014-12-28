#!/bin/sh

# 插入连续多天数据

CURR_PATH=`cd $(dirname $0);pwd;`
cd $CURR_PATH

start=$1
dur=$2
source=$3

for i in `seq $dur`
do
    lastday=`date -d "$start -1 days"`
    day=`date -d "$lastday +$i days" +%Y%m%d`
    python insert_task_monitor.py -d $day -s $source
    echo "$day insert ok!"
done
