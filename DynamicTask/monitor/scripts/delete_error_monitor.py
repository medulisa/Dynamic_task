#!/usr/bin/env python
#coding=utf-8

'''
@author: ZhangYang
@date: 2014.10.13
@brief: 更新error_task_monitor
'''

import sys
import db1
import time
import getopt


def delete_monitor_by_day(crawl_day):
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='monitor'
    try:
        sql = "DELETE FROM task_error_monitor WHERE workload_key like '%" + crawl_day + "'"
        db1.ExecuteSQL(host, user, passwd, db, sql)
    except Exception, e:
        print str(e)

def delete_monitor_by_code(error_code):
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='monitor'
    try:
        sql = "DELETE FROM task_error_monitor WHERE error_code = " + error_code
        db1.ExecuteSQL(host, user, passwd, db, sql)
    except Exception, e:
        print str(e)

if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], 'd:c:')
    for o, a in opts:
        if o == "-c":
            error_code = a
            delete_monitor_by_code(error_code)
        elif o == "-d":
            crawl_day = a
            delete_monitor_by_day(crawl_day)
