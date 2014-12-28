#ifndef _FLIGHT_GENERATOR_HPP
#define _FLIGHT_GENERATOR_HPP

/*
 *  AUTHOR: ZHANGYANG
 *  DATE: 2014.08.19
 *  BRIEF: 航班任务自动生成
 */

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "common/time/datetime.h"
#include "common/string/algorithm.h"
#include "common/service_log.hpp"
#include "BaseGenerator.hpp"
using namespace std;

typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > TASK_DATA;
typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > AIRPORT_DATA;
typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > CITY_DATA;
typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > FLIGHT_SOURCE;
typedef tr1::unordered_map< string, vector< pair<string, string> > > FLIGHT_PAIR;

class FlightGenerator : public BaseGenerator
{
    private:
        FlightGenerator();
        FlightGenerator(DateTime now, DateTime today);
        ~FlightGenerator();
    private:
        static FlightGenerator* m_pInstance;

    public:
        static FlightGenerator* getInstance(DateTime now, DateTime today)
        {
            if( NULL == m_pInstance)
                m_pInstance = new FlightGenerator(now, today);
            return m_pInstance;
        }

    public:
        /*
         * 计算一个任务的得分
         * @param: workload_key
         * @param: updatetime, 上次更新时间
         * @param: price_wave, 价格波动
         */
        float getTaskScore(const string& workload_key, const string& updatetime, float price_wave);
        
        /*
         * 从workload_key对应到content
         */
        string key2Content(const string& key);

        float getCityScore(const string& dept_city, const string& dest_city);
        float getAirportScore(const string& dept_id, const string& dest_id);
        float getSourceScore(const string& source);
        float getDayScore(const string& dept_day);
        float getDayIntervalScore(const string& dept_day);
        float getUpdateRewardScore(const string& updatetime);
    private:
        bool connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db);
        bool loadAirport(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadCity(const string&host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadFlightSource(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadFlightPair(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        string getContentFromRule(const string& rule, const string& spliter, const string& type, const string& dept_id, const string& dest_id);

    private:
        CITY_DATA m_city_map;
        AIRPORT_DATA m_airport_map;
        FLIGHT_SOURCE m_flight_source;
        FLIGHT_PAIR m_flight_pair;

        DateTime m_now;
        DateTime m_today;
};


#endif  // _FLIGHT_GENERATOR_HPP
