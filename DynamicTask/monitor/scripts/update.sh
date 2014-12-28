#!/bin/sh

CURR_PATH=`cd $(dirname $0);pwd;`
cd $CURR_PATH

DEL_DAY=`date -d "5 day" +%Y%m%d`
ADD_DAY=`date -d "70 day" +%Y%m%d`
# 删除过期数据
python delete_task_monitor.py ${DEL_DAY}
echo " delete ok!"
# 插入新数据
python insert_task_monitor.py -d ${ADD_DAY}
echo " insert ok!"
