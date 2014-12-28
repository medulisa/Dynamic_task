#!/usr/bin/env python
#coding=utf-8

'''
@author: ZhangYang
@date: 2014.09.05
@brief: 删除room_task_monitor和flight_task_monitor
'''

import sys
import db1
import time
import datetime


# delete monitor
def delete_monitor_by_day(crawl_day):
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='monitor'

    try:
        flight_sql = "DELETE FROM flight_task_monitor WHERE workload_key like '%" + crawl_day + "'"
        db1.ExecuteSQL(host, user, passwd, db, flight_sql)
        room_sql = "DELETE FROM room_task_monitor WHERE workload_key like '%" + crawl_day + "'"
        db1.ExecuteSQL(host, user, passwd, db, room_sql)
        train_sql = "DELETE FROM train_task_monitor WHERE workload_key like '%" + crawl_day + "'"
        db1.ExecuteSQL(host, user, passwd, db, train_sql)
    except Exception, e:
        print str(e)


if __name__ == '__main__':
    crawl_day=sys.argv[1]
    delete_monitor_by_day(crawl_day)
