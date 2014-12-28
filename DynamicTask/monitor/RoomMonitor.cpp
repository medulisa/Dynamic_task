#include "RoomMonitor.hpp"

RoomMonitor::RoomMonitor()
{
    BaseMonitor();
}

RoomMonitor::~RoomMonitor()
{
}

void RoomMonitor::free()
{
    CRAWL_DATA *p = new CRAWL_DATA;
    m_room_crawl_data.swap( *p );
    delete p;

    MONITOR_DATA *p1 = new MONITOR_DATA;
    m_room_monitor_data.swap( *p1 );
    delete p1;
}

bool RoomMonitor::readRoom()
{
    UseMysql mysql("127.0.0.1", "crawl", "root", "miaoji@2014!");
    if ( !mysql.connect())
    {
        return false;
    }

    // 读取room表
    // 城市, 源, 源酒店id, 源房间id, 入住日期, 离店日期, 价格, 更新时间
    string sql = "SELECT city, source, source_hotelid, source_roomid, check_in, check_out, price, update_time FROM room ORDER BY city, check_in, check_out, source, source_hotelid";
    if (!mysql.query(sql))
    {
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql.use_result();
        int num_fields = mysql.num_fields(res);
        if(num_fields != ROOM_NUM_FIELDS)
        {
            _ERROR("READ room, wrong num fields : %d!", num_fields);
            return false;
        }
        MYSQL_ROW row;
        if(res)
        {
            string last_source;
            string last_city;
            string last_source_hotelid;
            string last_checkin_day; 
            string last_checkout_day;
            string last_updatetime;
            Json::Value price_value;
            while( row = mysql.fetch_row(res) )
            {
                string city = row[0];
                string source = row[1];
                string source_hotelid = row[2];
                string source_roomid = row[3];
                string checkin_day = row[4];
                string checkout_day = row[4];
                if (source == "ctrip")
                    continue;
                if (checkin_day == "NULL" || checkout_day == "NULL")
                    continue;
                stripDay(checkin_day);
                stripDay(checkout_day);
                string price = row[6];
                string updatetime = row[7];
                bool flag = false;      // 标示该条记录和上一条记录是否属于同一个workload
                // 对于按城市抓取的源和按酒店抓取的源分别判断该条记录和上一条记录是否属于同一个workload
                if(source == "biyi" || source == "agoda")
                {
                    if(city == last_city && source == last_source && checkin_day == last_checkin_day && checkout_day == last_checkout_day)
                    {
                        flag = true;
                    }
                }
                else
                {
                    if(source_hotelid == last_source_hotelid && source == last_source && checkin_day == last_checkin_day && checkout_day == last_checkout_day)
                    {
                        flag = true;
                    }
                }
                //
                if(flag)
                {
                    if(source_roomid == "NULL")
                    {
                        price_value["room_num"] = price_value.get("room_num", 0).asInt() + 1;
                        price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + atof(price.c_str());
                    }
                    else
                    {
                        price_value["room_num"] = price_value.get("room_num", 0).asInt() + 1;
                        price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + atof(price.c_str());
                        //price_value[source_roomid] = atof(price.c_str());
                    }
                }
                else
                {
                    // 价格序列化
                    string price_str = serializePrice(price_value);
                    price_value.clear();
                    // 读入该条任务执行情况
                    vector<string> vec;
                    vec.push_back(last_city);
                    vec.push_back(last_source);
                    vec.push_back(last_source_hotelid);
                    vec.push_back(last_checkin_day);
                    vec.push_back(last_checkout_day);
                    vec.push_back(price_str);
                    vec.push_back(last_updatetime);
                    // 插入到crawl数据
                    if(last_city != "")
                        m_room_crawl_data.push_back(vec);
                    // 记录新任务
                    if(source_roomid == "NULL")
                    {
                        price_value["room_num"] = price_value.get("room_num", 0).asInt() + 1;
                        price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + atof(price.c_str());
                    }
                    else
                    {
                        price_value["room_num"] = price_value.get("room_num", 0).asInt() + 1;
                        price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + atof(price.c_str());
                        //price_value[source_roomid] = atof(price.c_str());
                    }
                    
                }
                // 更新为上一条记录
                last_city = city;
                last_source = source;
                last_source_hotelid = source_hotelid;
                last_checkin_day = checkin_day;
                last_checkout_day = checkout_day;
                last_updatetime = updatetime;
            }

        }
        mysql.free_result(res);
    }
    _INFO("read room ok!");
    return true;
}


bool RoomMonitor::writeRoom()
{
    UseMysql mysql(HOST, "monitor", USR, PASSWD);
    string sql = "select workload_key, source, last_updatetime, last_price, updatetime, price, price_wave from room_task_monitor";
    if( !readMonitorData(m_room_monitor_data, sql, ROOM_MONITOR_NUM_FIELDS, HOST, "monitor", USR, PASSWD) )
    {
        _ERROR("IN writeRoom, CANNOT read monitor data!");
        return false;
    }
    
    // 更新现有监控数据
    MONITOR_DATA diff_monitor_data;
    CRAWL_DATA::iterator it = m_room_crawl_data.begin();
    for(; it != m_room_crawl_data.end(); ++it)
    {
        vector<string> crawl_vec = *it;
        string workload_key;
        if( ROOM_NUM_FIELDS-1 == crawl_vec.size() )
        {
            string city = crawl_vec[0];
            string source = crawl_vec[1];
            string source_hotelid = crawl_vec[2];
            string checkin_day = crawl_vec[3];
            string checkout_day = crawl_vec[4];
            string price_str = crawl_vec[5];
            string updatetime = crawl_vec[6];
            // 生成workload_key
            workload_key = m_key_generator->getRoomKey(source, city, source_hotelid, checkin_day, checkout_day);
            // 插入
            insertMonitorData(m_room_monitor_data, diff_monitor_data, workload_key, source, updatetime, price_str);
        }
        else
        {
            _ERROR("In writeRoom, crawl_vec size is not supposed!");
            continue;
        }
    }
    
    // 生成更新sql
    string sql2 = "REPLACE INTO room_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES ";
    // 监控数据写回数据库
    if (!updateMonitorData(diff_monitor_data, HOST, "monitor", USR, PASSWD, sql2))
    {
        _INFO("UPDATE room_task_monitor failed!");
        return false;
    }
    _INFO("UPDATE room_task_monitor ok!"); 

    return true;
}
