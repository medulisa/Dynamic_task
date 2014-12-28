#ifndef _HOTEL_GENERATOR_HPP
#define _HOTEL_GENERATOR_HPP

/*
 *  AUTHOR: ZHANGYANG
 *  DATE: 2014.08.19
 *  BRIEF: 酒店任务自动生成
 */

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include <tr1/unordered_map>
#include "json/json.h"
#include "common/time/datetime.h"
#include "common/string/algorithm.h"
#include "common/service_log.hpp"
#include "BaseGenerator.hpp"
using namespace std;

typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > TASK_DATA;
typedef tr1::unordered_map< string, float > HOTEL_SCORE;
typedef tr1::unordered_map< string, float > CITY_SCORE;

class HotelGenerator : public BaseGenerator
{
    private:
        HotelGenerator();
        HotelGenerator(DateTime now, DateTime today);
        ~HotelGenerator();
    private:
        static HotelGenerator* m_pInstance;

    public:
        static HotelGenerator* getInstance(DateTime now, DateTime today)
        {
            if(NULL == m_pInstance)
                m_pInstance = new HotelGenerator(now, today);
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

        // 城市得分
        float getCityScore(const string& city_en);
        // 酒店得分
        float getHotelScore(const string& uid);
        // 入住日期得分，比如圣诞，元旦等假期
        float getCheckinDayScore(const string& checkin_day);
        // 入住日期距离当前间隔的得分
        float getDayIntervalScore(const string& checkin_day);
        // 入住天数的得分
        float getDaysScore(int days);
        // 源加权
        float getSourceScore(const string& source);
        // 更新时间加权
        float getUpdateRewardScore(const string& updatetime);

    private:
        float getSourceLevelScore(const string& source);
        float getSourceRewardScore(const string& source);
        string getHotelID(const string& source, const string& source_id);

        bool connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db);
        bool loadCityLevel(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadHotelID(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadHotelStar(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadHotelGrade(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        bool loadHotelComment(const string& host, const string& user, const string& passwd, const string& db, const string& table);
        // 载入hotel workload， 对应key和content
        bool loadHotelWorkload(const string& host, const string& user, const string& passwd, const string& db, const string& table);

    private:
        CITY_SCORE m_city_level_map;
        HOTEL_SCORE m_hotel_comment_map;
        HOTEL_SCORE m_hotel_grade_map;
        HOTEL_SCORE m_hotel_star_map;
        tr1::unordered_map<string, string> m_hotel_id_map;
        tr1::unordered_map<string, string> m_key_to_content;
        
        DateTime m_now;
        DateTime m_today;
};


#endif  // _HOTEL_GENERATOR_HPP
