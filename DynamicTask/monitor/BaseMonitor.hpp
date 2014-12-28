#ifndef _BASE_MONITOR_HPP_
#define _BASE_MONITOR_HPP_ 

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <tr1/unordered_map>
#include "json/json.h"
#include "common/service_log.hpp"
#include "CommonFuc.hpp"
#include "key/KeyGenerator.hpp"
#include "UseMysql.hpp"

#define HOST "10.66.115.222"
#define USR "root"
#define PASSWD "miaoji@2014!"

typedef vector< vector<string> > CRAWL_DATA;
typedef tr1::unordered_map< string, vector<string> > MONITOR_DATA; 

using namespace std;

class BaseMonitor
{
    public:
        BaseMonitor();
        ~BaseMonitor();

        void init();
    protected:
        bool createMonitorTable(const string& host, const string& db, const string& usr, const string& passwd);

        string serializePrice(const Json::Value& price_value);

        void insertMonitorData(const MONITOR_DATA& monitor_data, MONITOR_DATA& diff_monitor_data, const string& workload_key, const string& source, const string & updatetime, const string& price_str);

        float getPriceWave(const string& last_price_str, const string& price_str);

        bool parsePrice(const string& price_str, Json::Value& price_value);

        bool readMonitorData(MONITOR_DATA& monitor_data, const string& sql, int num_fields, const string& host, const string& db, const string& usr, const string& passwd);

        bool updateMonitorData(const MONITOR_DATA& diff_monitor_data, const string& host, const string& db, const string& usr, const string& passwd, string& sql);

        inline void stripDay(string& day)
        {
            string year = day.substr(0, 4);
            string month = day.substr(5, 2);
            string date = day.substr(8, 2);
            day = year + month + date;
        }

    protected:
        KeyGenerator* m_key_generator;
};

#endif  /*_BASE_MONITOR_HPP_*/
