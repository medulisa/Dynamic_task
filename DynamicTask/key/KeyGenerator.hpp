#ifndef _KEY_GENERATOR_HPP
#define _KEY_GENERATOR_HPP

/*
 * AUTHOR: ZHANGYANG
 * DATE: 2014.08.13
 * BRIEF: workload_key生成器，包括flight oneway， flight round， room
 */

#include <string>
#include <vector>
#include <tr1/unordered_map>
#include <mysql/mysql.h>
#include "common/time/datetime.h"
#include "common/string/algorithm.h"
using namespace std;

typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > FLIGHT_DATA;
//typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > TRAIN_DATA;
typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > CITY_DATA;

class KeyGenerator
{
    public:
        KeyGenerator();
        ~KeyGenerator();

        string getFlightOnewayKey(const string& source, const string& dept_id, const string& dest_id, const string& dept_day);
        string getFlightRoundKey(const string& source, const string& dept_id, const string& dest_id, const string& dept_day, const string& dest_day);
        
        /*  获得酒店workload_key
         *  @param: source, 源
         *  @param: city_name, 城市中文名
         *  @param: hotel_id, 源酒店id
         *  @param: checkin_day, 入住日期
         *  @param: checkout_day, 离店日期
         */
        string getRoomKey(const string& source, const string& city_name, const string& hotel_id, const string& checkin_day, const string& checkout_day);

        string getTrainKey(const string& source, const string& dept_id, const string& dest_id, const string& dept_day);

    private:
        bool connect2DB(MYSQL* mysql, const string& host, const string& db, const string& user, const string& passwd);
        
        // 载入航班源数据，m_flight_source
        bool loadFlightSource();

        //载入火车源数据，m_train_source
//        bool loadTrainSource();
        
        // 载入城市数据，m_city
        bool loadCity();
    
    private:
        FLIGHT_DATA m_flight_source;        // 航班源信息, key为source，比如expidia
//暂时不需要        TRAIN_DATA m_train_source;          // 车次源信息，key为source
        CITY_DATA m_city;                   // 城市数据， key为机场三字码
};

#endif //  _KEY_GENERATOR_HPP
