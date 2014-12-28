#/bin/sh

CURR_PATH=`cd $(dirname $0);pwd;`
cd $CURR_PATH


kill -9 monitor
sleep 1

sh ./start.sh
