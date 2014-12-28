#include "TrainMonitor.hpp"

bool TrainMonitor::readTrain()
{
    UseMysql mysql(HOST, "crawl", USR, PASSWD);
    if ( !mysql.connect() )
    {
        return false;
    }

    // 读取train_new表
    // 车次号，出发城市，到达城市，出发日期，价格，更新时间, 源
    string sql = "select train_no, dept_city, dest_city, dept_day, price, tax, update_time, source from train_new ORDER BY dept_city, dest_city, dept_day, source";
    if (!mysql.query(sql))
    {
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql.use_result();
        int num_fields = mysql.num_fields(res);
        if(num_fields != TRAIN_NUM_FIELDS)
        {
            _ERROR("READ train, wrong num fields : %d!", num_fields);
            return false;
        }
        MYSQL_ROW row;
        if(res)
        {
            string last_dept_city;
            string last_dest_city;
            string last_dept_day;
            string last_source = "";
            string last_updatetime;
            Json::Value price_value;
            while( row = mysql.fetch_row(res) )
            {
                string train_no = row[0];
                string dept_city = row[1];
                string dest_city = row[2];
                string dept_day = row[3];
                stripDay(dept_day);
                string price = row[4];
                float price_f = atof(price.c_str());
                string tax = row[5];
                float tax_f = ( tax=="-1" ? 0 : atof(tax.c_str()) );
                string updatetime = row[6];
                string raw_source = row[7];
                string source = raw_source.substr( 0, raw_source.find("::") );   

                // 判断该记录和上条记录是否属于同一个task
                if( dept_city == last_dept_city && dest_city == last_dest_city && dept_day == last_dept_day && source == last_source)
                {
                    price_value["train_num"] = price_value.get("train_num", 0).asInt() + 1;
                    price_value["price_all"] = price_value.get("price_all", 0.0).asDouble() + price_f + tax_f;
                }
                else
                {
                    // 价格序列化
                    string price_str = serializePrice(price_value);
                    // 读入该条任务执行情况
                    vector<string> vec;
                    vec.push_back(last_dept_city);
                    vec.push_back(last_dest_city);
                    vec.push_back(last_dept_day);
                    vec.push_back(price_str);
                    vec.push_back(last_updatetime);
                    vec.push_back(last_source);
                    if(last_source != "")
                        m_train_crawl_data.push_back(vec);
                    // 清空价格
                    price_value.clear();
                    // 记录新任务
                    //price_value[train_no] = atof(price.c_str());
                    price_value["train_num"] = 1;
                    price_value["price_all"] = price_f + tax_f;
                }
                // 更新任务指示器
                last_dept_city = dept_city;
                last_dest_city = dest_city;
                last_dept_day = dept_day;
                last_source = source;
                last_updatetime = updatetime;
            }
            // 最后一个workload 更新
            // TODO
        }
        mysql.free_result(res);
    }
    _INFO("read train ok!");
    return true;
}


bool TrainMonitor::writeTrain()
{
    UseMysql mysql(HOST, "monitor", USR, PASSWD);
    string sql = "select workload_key, source, last_updatetime, last_price, updatetime, price, price_wave from train_task_monitor";
    if( !readMonitorData(m_train_monitor_data, sql, TRAIN_MONITOR_NUM_FIELDS, HOST, "monitor", USR, PASSWD) )
    {
        _ERROR("IN writeTrain, CANNOT read monitor data!");
        return false;
    }
    
    // 更新现有监控数据
    MONITOR_DATA diff_monitor_data;
    CRAWL_DATA::iterator it = m_train_crawl_data.begin();
    for(; it != m_train_crawl_data.end(); ++it)
    {
        vector<string> crawl_vec = *it;
        string workload_key;
        if( TRAIN_NUM_FIELDS - 2 == crawl_vec.size() )
        {   
            string source = crawl_vec[5];
            string dept_city = crawl_vec[0];
            string dest_city = crawl_vec[1];
            string dept_day = crawl_vec[2];
            string price_str = crawl_vec[3];
            string updatetime = crawl_vec[4];
            // 生成workload_key
            workload_key = m_key_generator->getTrainKey(source, dept_city, dest_city, dept_day);
            // 插入
            insertMonitorData(m_train_monitor_data, diff_monitor_data, workload_key, source, updatetime, price_str);
        }
        else
        {
            _ERROR("In writeTrain, crawl_vec size is not supposed!");
            continue;
        }
    }
    
    // 生成更新sql
    string sql2 = "REPLACE INTO train_task_monitor (workload_key, source, last_updatetime, last_price, updatetime, price, price_wave) VALUES ";
    // 监控数据写回数据库
    if (!updateMonitorData(diff_monitor_data, HOST, "monitor", USR, PASSWD, sql2))
    {
        _INFO("UPDATE train_task_monitor failed!");
        return true;
    }
    _INFO("UPDATE train_task_monitor ok!"); 

    return true;
}








