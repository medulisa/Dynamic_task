#ifndef _BASE_GENERATOR_HPP
#define _BASE_GENERATOR_HPP

/*
 *  AUTHOR: ZHANGYANG
 *  DATE: 2014.08.30
 *  BRIEF: 任务生成基类
 */

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include <tr1/unordered_map>
#include "json/json.h"
using namespace std;

class BaseGenerator
{
    public:
        BaseGenerator() {}
        virtual ~BaseGenerator() {}

    public:
        /*
         * 计算一个任务的得分
         * @param: workload_key
         * @param: updatetime, 上次更新时间
         * @param: price_wave, 价格波动
         */
        virtual float getTaskScore(const string& workload_key, const string& updatetime, float price_wave) {return 0.0;}

        /*
         * workload_key转化为content
         * @param: 任务key
         * @return: 爬虫识别的任务内容
         */
        virtual string key2Content(const string& workload_key) {}
};


#endif  // _HOTEL_GENERATOR_HPP
