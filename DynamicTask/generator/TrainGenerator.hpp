#ifndef _TRAIN_GENERATOR_HPP
#define _TRAIN_GENERATOR_HPP

/*
 *  AUTHOR: ZHANGYANG
 *  DATE: 2014.10.09
 *  BRIEF: 火车任务自动生成
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

class TrainGenerator : public BaseGenerator
{
    private:
        TrainGenerator();
        TrainGenerator(DateTime now, DateTime today);
        ~TrainGenerator();
    private:
        static TrainGenerator* m_pInstance;

    public:
        static TrainGenerator* getInstance(DateTime now, DateTime today)
        {
            if(NULL == m_pInstance)
                m_pInstance = new TrainGenerator(now, today);
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

        // 更新时间加权
        float getUpdateRewardScore(const string& updatetime);

    private:
        bool connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db);

    private:
        tr1::unordered_map<string, string> m_key_to_content;
        
        DateTime m_now;
        DateTime m_today;
};


#endif  // _TRAIN_GENERATOR_HPP
