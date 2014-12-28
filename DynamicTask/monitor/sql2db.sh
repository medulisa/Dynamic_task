#!/bin/sh

#NOW=`date +%Y%m%d_%H%M`
#FILENAME=$NOW.sql

HOTEL_FILE='/data/crawl_data/release/room.sql'
FLIGHT_ONE_FILE='/data/crawl_data/release/flight_new.sql'
FLIGHT_ROUND_FILE='/data/crawl_data/release/flight_round.sql'
TRAIN_FILE='/data/crawl_data/release/train_new.sql'

sleep 10

echo "begin scp!"
scp root@10.131.136.190:$FLIGHT_ONE_FILE /home/workspace/DynamicTask/monitor/sql
scp root@10.131.136.190:$FLIGHT_ROUND_FILE /home/workspace/DynamicTask/monitor/sql
scp root@10.131.136.190:$HOTEL_FILE /home/workspace/DynamicTask/monitor/sql
scp root@10.131.136.190:$TRAIN_FILE /home/workspace/DynamicTask/monitor/sql
echo "after scp!"

# 导库
mysql -uroot -pmiaoji@2014! crawl < /home/workspace/DynamicTask/monitor/sql/flight_new.sql
mysql -uroot -pmiaoji@2014! crawl < /home/workspace/DynamicTask/monitor/sql/flight_round.sql
mysql -uroot -pmiaoji@2014! crawl < /home/workspace/DynamicTask/monitor/sql/room.sql
mysql -uroot -pmiaoji@2014! crawl < /home/workspace/DynamicTask/monitor/sql/train_new.sql
