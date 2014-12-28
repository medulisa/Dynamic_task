#!/bin/bash

ulimit -c unlimited
ulimit -f unlimited

CURR_PATH=`cd $(dirname $0);pwd;`
cd $CURR_PATH

#while(true)
#do
#    pids=`ps -ef | grep "generator" | wc -l`
#    if [ $pids != 1 ];
#    then
#        echo "there's generator process! please wait!"
#        sleep 20
#    else
#        break
#    fi
#done

./generator 1> log 2>&1 &
