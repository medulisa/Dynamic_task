#!/bin/bash

CURR_PATH=`cd $(dirname $0);pwd;`
cd $CURR_PATH

sh ./sql2db.sh
nohup ./monitor 1> log 2>&1 &
