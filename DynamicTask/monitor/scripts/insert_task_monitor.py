#!/usr/bin/env python
#coding=utf-8

'''
@author: ZhangYang
@date: 2014.09.05
@brief: 更新room_task_monitor和flight_task_monitor
'''

import db1
import time
import datetime
import sys
import getopt

FLIGHT_KEY = []
HOTEL_KEY = []
TRAIN_KEY = []

# 读取长期表的workload_key
def read_longterm_key(crawl_day, source):
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='workload'
    sql = ''
    if crawl_day == 'None' and source == 'None':
        sql = 'SELECT workload_key, source FROM workload_longterm where timeslot = -1'
    elif crawl_day == 'None' and source != 'None':
        sql = "SELECT workload_key, source FROM workload_longterm where source = '" + source + "'" + " and timeslot = -1"
    elif crawl_day != 'None' and source == 'None':
        sql = 'SELECT workload_key, source FROM workload_longterm where crawl_day = ' + crawl_day + " and timeslot = -1"
    else:
        sql = 'SELECT workload_key, source FROM workload_longterm where crawl_day = ' + crawl_day + " and source = '" + source + "'" + " and timeslot = -1"
    results = db1.QueryBySQL(host, user, passwd, db, sql)
    try:
        for result in results:
            key = result['workload_key']
            source = result['source']
            if key.find('Flight') != -1:
                source = source[0:-6]
                FLIGHT_KEY.append((key, source))
            elif key.find('Hotel') != -1:
                source = source[0:-5]
                HOTEL_KEY.append((key, source))
            elif key.find('Rail') != -1:
                source = source[0:-4]
                TRAIN_KEY.append((key, source))
    except Exception, e:
        print str(e)

# 读取static表的workload_key
def read_static_key(crawl_day, source):
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='workload'
    sql = ''
    if crawl_day == 'None' and source == 'None':
        sql = 'SELECT workload_key, source FROM workload_static'
    elif crawl_day == 'None' and source != 'None':
        sql = "SELECT workload_key, source FROM workload_static where source = '" + source + "'"
    elif crawl_day != 'None' and source == 'None':
        sql = 'SELECT workload_key, source FROM workload_static where crawl_day = ' + crawl_day
    else:
        sql = 'SELECT workload_key, source FROM workload_static where crawl_day = ' + crawl_day + " and source = '" + source + "'"
    results = db1.QueryBySQL(host, user, passwd, db, sql)
    try:
        for result in results:
            key = result['workload_key']
            source = result['source']
            if key.find('Flight') != -1:
                source = source[0:-6]
                FLIGHT_KEY.append((key, source))
            elif key.find('Hotel') != -1:
                source = source[0:-5]
                HOTEL_KEY.append((key, source))
            elif key.find('Rail') != -1:
                source = source[0:-4]
                TRAIN_KEY.append((key, source))
    except Exception, e:
        print str(e)

# 将key插入monitor
def insert_monitor_key(crawl_day, source):
    read_longterm_key(crawl_day, source)
    read_static_key(crawl_day, source)
    
    host='10.66.115.222'
    user='root'
    passwd='miaoji@2014!'
    db='monitor'

    print 'FLIGHT_KEY size is %d, HOTEL_KEY size is %d, TRAIN_KEY size is %d\n' % (len(FLIGHT_KEY), len(HOTEL_KEY), len(TRAIN_KEY))
    FLIGHT_KEY1 = list(set(FLIGHT_KEY))
    HOTEL_KEY1 = list(set(HOTEL_KEY))
    TRAIN_KEY1 = list(set(TRAIN_KEY))
    print 'FLIGHT_KEY1 size is %d, HOTEL_KEY1 size is %d, TRAIN_KEY1 size is %d\n' % (len(FLIGHT_KEY1), len(HOTEL_KEY1), len(TRAIN_KEY1))
    
    # 插入flight key
    try:
        flight_data = [(i[0], i[1], 'NULL', 'NULL', 'NULL', 'NULL', 0.999) for i in FLIGHT_KEY1 ]
        print 'read flight_data ok!'
        sql = 'INSERT ignore INTO flight_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES (%s, %s, %s, %s, %s, %s, %s)'
        db1.ExecuteSQLs(host, user, passwd, db, sql, flight_data)
    except Exception, e:
        print str(e)

    # 插入hotel key
    try:
        room_data = [(i[0], i[1], 'NULL', 'NULL', 'NULL', 'NULL', 0.999) for i in HOTEL_KEY1 ]
        print 'read room_data ok!'
        sql = 'INSERT ignore INTO room_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES (%s, %s, %s, %s, %s, %s, %s)'
        db1.ExecuteSQLs(host, user, passwd, db, sql, room_data)
    except Exception, e:
        print str(e)

    # 插入train key
    try:
        train_data = [(i[0], i[1], 'NULL', 'NULL', 'NULL', 'NULL', 0.999) for i in TRAIN_KEY1 ]
        print 'read train_data ok!'
        sql = 'INSERT ignore INTO train_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES (%s, %s, %s, %s, %s, %s, %s)'
        db1.ExecuteSQLs(host, user, passwd, db, sql, train_data)
    except Exception, e:
        print str(e)


if __name__ == '__main__':
    crawl_day = 'None'
    source = 'None'
    opts, args = getopt.getopt(sys.argv[1:], 'd:s:')
    for o, a in opts:
        if o == "-d":
            crawl_day = a
        elif o == "-s":
            source = a
    insert_monitor_key(crawl_day, source)
