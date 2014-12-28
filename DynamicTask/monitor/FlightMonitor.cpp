#include "FlightMonitor.hpp"

FlightMonitor::FlightMonitor()
{
}

FlightMonitor::~FlightMonitor()
{
}

void FlightMonitor::free()
{
    // 释放crawl_data
    CRAWL_DATA *p = new CRAWL_DATA;
    m_flight_crawl_data.swap( *p );
    delete p;

    // 释放monitor_data
    MONITOR_DATA *p1 = new MONITOR_DATA;
    m_flight_monitor_data.swap( *p1 );
    delete p1;
}

bool FlightMonitor::readFlightOneway()
{
    UseMysql mysql("127.0.0.1", "crawl", "root", "miaoji@2014!");
    if ( !mysql.connect())
    {
        return false;
    }

    // 读取flight表
    // 航班号，出发城市三字码，到达城市三字码，出发日期，价格，更新时间, 源
    string sql = "select flight_no, dept_id, dest_id, dept_day, price, tax, updatetime, source from flight_new ORDER BY dept_id, dest_id, dept_day, source";
    if (!mysql.query(sql))
    {
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql.use_result();
        int num_fields = mysql.num_fields(res);
        if(num_fields != FLIGHT_ONEWAY_NUM_FIELDS)
        {
            _ERROR("READ flight, wrong num fields : %d!", num_fields);
            return false;
        }
        MYSQL_ROW row;
        if(res)
        {
            string last_dept_id;
            string last_dest_id;
            string last_dept_day;
            string last_source = "";
            string last_updatetime;
            Json::Value price_value;
            while( row = mysql.fetch_row(res) )
            {
                string flight_no = row[0];
                string dept_id = row[1];
                string dest_id = row[2];
                string dept_day = row[3];
                stripDay(dept_day);
                string price = row[4];
                float price_f = atof(price.c_str());
                string tax = row[5];
                float tax_f = ( tax=="-1" ? 0 : atof(tax.c_str()) );
                string updatetime = row[6];
                string raw_source = row[7];
                string source = raw_source.substr( 0, raw_source.find("::") );    // 解析出expidia

                // 判断该记录和上条记录是否属于同一个task
                if( dept_id == last_dept_id && dest_id == last_dest_id && dept_day == last_dept_day && source == last_source)
                {
                    price_value["flight_num"] = price_value.get("flight_num", 0).asInt() + 1;
                    price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + price_f + tax_f;
                }
                else
                {
                    // 价格序列化
                    string price_str = serializePrice(price_value);
                    // 读入该条任务执行情况
                    vector<string> vec;
                    vec.push_back(last_dept_id);
                    vec.push_back(last_dest_id);
                    vec.push_back(last_dept_day);
                    vec.push_back(price_str);
                    vec.push_back(last_updatetime);
                    vec.push_back(last_source);
                    if(last_source != "")
                        m_flight_crawl_data.push_back(vec);
                    // 清空价格
                    price_value.clear();
                    // 记录新任务
                    //price_value[flight_no] = atof(price.c_str());
                    price_value["flight_num"] = 1;
                    price_value["price_all"] = price_f + tax_f;
                }
                // 更新任务指示器
                last_dept_id = dept_id;
                last_dest_id = dest_id;
                last_dept_day = dept_day;
                last_source = source;
                last_updatetime = updatetime;
            }
            // 最后一个workload 更新
            // TODO
        }
        mysql.free_result(res);
    }
    _INFO("read flight oneway ok!");
    return true;
}


bool FlightMonitor::readFlightRound()
{
    UseMysql mysql("127.0.0.1", "crawl", "root", "miaoji@2014!");
    if( !mysql.connect() )
    {   
        return false;
    }
    
    // 读取flight_round表
    // 出发航班号，出发城市三字码，归来航班号，到达城市三字码，出发日期，归来日期， 价格，税, 更新时间, 源
    string sql = "select flight_no_A, dept_id, flight_no_B, dest_id, dept_day, dest_day, price, tax, updatetime, source from flight_round ORDER BY dept_id, dest_id, dept_day, dest_day, source";
    if(!mysql.query(sql))
    {   
        return false;
    }   
    else
    {   
        MYSQL_RES* res = mysql.use_result();
        int num_fields = mysql.num_fields(res);
        if(num_fields != FLIGHT_ROUND_NUM_FIELDS)
        {
            _ERROR("READ flight_round, wrong num fields: %d!", num_fields);
            return false;
        }
        MYSQL_ROW row;
        if (res)
        {
            string last_dept_id;
            string last_dest_id;
            string last_dept_day;
            string last_dest_day;
            string last_source;
            string last_updatetime;
            Json::Value price_value;
            while( row = mysql.fetch_row(res) )
            {
                string flight_no_A = row[0];
                string dept_id = row[1];
                string flight_no_B = row[2];
                string dest_id = row[3];
                string dept_day = row[4];
                stripDay(dept_day);
                string dest_day = row[5];
                stripDay(dest_day);
                string price = row[6];
                float price_f = atof(price.c_str());
                string tax = row[7];
                float tax_f = (tax == "-1" ? 0 : atof(tax.c_str()) );
                string updatetime = row[8];
                string raw_source = row[9];
                string source = raw_source.substr( 0, raw_source.find("::") ) + "Round";         // 解析出jijitongRound
                // 判断该记录和上条记录是否属于同一个task
                if( dept_id == last_dept_id && dest_id == last_dest_id && dept_day == last_dept_day && dest_day == last_dest_day && source == last_source)
                {
                    price_value["flight_num"] = price_value.get("flight_num", 0).asInt() + 1;
                    price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + price_f + tax_f;
                }
                else
                {
                    // 价格序列化
                    string price_str = serializePrice(price_value);
                    // 读入该条任务执行情况
                    vector<string> vec;
                    vec.push_back(last_dept_id);
                    vec.push_back(last_dest_id);
                    vec.push_back(last_dept_day);
                    vec.push_back(last_dest_day);
                    vec.push_back(price_str);
                    vec.push_back(last_updatetime);
                    vec.push_back(last_source);
                    if(last_source != "")
                        m_flight_crawl_data.push_back(vec);
                    // 清空价格
                    price_value.clear();
                    // 记录新任务
                    price_value["flight_num"] = 1;
                    price_value["price_all"] = price_f + tax_f;
                }
                // 更新任务指示器
                last_dept_id = dept_id;
                last_dest_id = dest_id;
                last_dept_day = dept_day;
                last_dest_day = dest_day;
                last_source = source;
                last_updatetime = updatetime;
            }
            // 最后一个workload 更新
            // TODO
        }
        mysql.free_result(res);
    }
    _INFO("read flight round ok!");
    return true;
}


bool FlightMonitor::writeFlight()
{
    UseMysql mysql(HOST, "monitor", USR, PASSWD);
    string sql = "select workload_key, source, last_updatetime, last_price, updatetime, price, price_wave from flight_task_monitor";
    if( !readMonitorData(m_flight_monitor_data, sql, FLIGHT_MONITOR_NUM_FIELDS, HOST, "monitor", USR, PASSWD) )
    {
        _ERROR("IN writeFlight, CANNOT read monitor data!");
        return false;
    }
    
    // 更新现有监控数据
    MONITOR_DATA diff_monitor_data;
    CRAWL_DATA::iterator it = m_flight_crawl_data.begin();
    for(; it != m_flight_crawl_data.end(); ++it)
    {
        vector<string> crawl_vec = *it;
        string workload_key;
        if( FLIGHT_ONEWAY_NUM_FIELDS - 2 == crawl_vec.size() )
        {   
            // 单程航班
            string source = crawl_vec[5];
            string dept_id = crawl_vec[0];
            string dest_id = crawl_vec[1];
            string dept_day = crawl_vec[2];
            string price_str = crawl_vec[3];
            string updatetime = crawl_vec[4];
            // 生成workload_key
            workload_key = m_key_generator->getFlightOnewayKey(source, dept_id, dest_id, dept_day);
            // 插入
            insertMonitorData(m_flight_monitor_data, diff_monitor_data, workload_key, source, updatetime, price_str);
        }
        else if( FLIGHT_ROUND_NUM_FIELDS-3 == crawl_vec.size() )
        {
            // 往返航班
            string dept_id = crawl_vec[0];
            string dest_id = crawl_vec[1];
            string dept_day = crawl_vec[2];
            string dest_day = crawl_vec[3];
            string price_str = crawl_vec[4];
            string updatetime = crawl_vec[5];
            string source = crawl_vec[6];
            // 生成workload_key
            workload_key = m_key_generator->getFlightRoundKey(source, dept_id, dest_id, dept_day, dest_day);
            // 插入
            insertMonitorData(m_flight_monitor_data, diff_monitor_data, workload_key, source, updatetime, price_str);
        }
        else
        {
            _ERROR("In writeFlight, crawl_vec size is not supposed!");
            continue;
        }
    }
    
    // 生成更新sql
    string sql2 = "REPLACE INTO flight_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES ";
    // 监控数据写回数据库
    if (!updateMonitorData(diff_monitor_data, HOST, "monitor", USR, PASSWD, sql2))
    {
        _INFO("UPDATE flight_task_monitor failed!");
        return false;
    }
    _INFO("UPDATE flight_task_monitor ok!"); 

    return true;
}
