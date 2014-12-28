#ifndef _TASK_GENERATOR_HPP
#define _TASK_GENERATOR_HPP

/*
 *  AUTHOR: ZHANGYANG
 *  DATE: 2014.08.19
 *  BRIEF: TASK自动生成，根据task的监控信息，智能生成下一个5分钟的任务
 */

#include <string>
#include <vector>
#include <algorithm>
#include <mysql/mysql.h>
#include <omp.h>
#include <pthread.h>
#include "json/json.h"
#include "common/time/datetime.h"
#include "common/string/algorithm.h"
#include "common/service_log.hpp" 
#include "HotelGenerator.hpp"
#include "FlightGenerator.hpp"
#include "TrainGenerator.hpp"
#include "Valve.hpp"
using namespace std;


// 时间槽间隔，单位：分钟
#define INTERVAL 5

enum Type
{
    None=0,
    Flight=1, 
    Hotel=2,
    Train=3
};

enum Mode
{
    Regular=1,
    Longterm=2
};


typedef tr1::unordered_map< string, tr1::unordered_map<string, string> > TYPE_DATA;
typedef tr1::unordered_map< int, TYPE_DATA > DATA;
typedef tr1::unordered_map< string, float > TASK;
typedef pair<string, float> PAIR;

bool cmp(const PAIR& x, const PAIR& y)
{
    return x.second > y.second;
}


class TaskGenerator
{
    public:
        TaskGenerator();
        ~TaskGenerator();
        
        /*
         *  生成下个时间槽的任务，并写入数据库
         *  @param: host, 数据库主机
         *  @param: user, 用户
         *  @param: passwd, 密码
         *  @param: db, 数据库
         */
        bool writeTask2DB(const string& host, const string& user, const string& passwd, const string& db, bool is_del=true);
        
        /*
         * 生成指定时间槽的任务
         * @param：timeslot, 待生成任务的时间槽
         */
        bool assignTaskByTimeslot(int timeslot);
        
        /*
         * 根据类别，按照重要度得到特定数量的任务
         * @param: type_data, 特定类别的任务数据，包含workload_key, source, wave等
         * @param: in_tasks, 输入的任务分数
         * @param: out_tasks, 生成的任务
         * @param: type, 类别
         * @param: count, 返回的任务数量
         */
        bool getTasks(const TYPE_DATA& type_data, TASK in_tasks, TASK& out_tasks, Type type, Mode mode);
        
        /*
         * 根据类别，返回特定数量的长期任务
         * @param: type_data, 特定类别的任务数据
         * @param: tasks, 生成的任务
         * @param: type, 类别
         * @param: count, 返回的任务数量
         */
        bool getLongtermTasks(const TYPE_DATA& type_data, TASK& tasks, Type type, Mode mode);

        bool writeTmpTasks();
        bool assignTmpTask(int timeslot);

        /*
         * 获取任务的惩罚分数，如不能执行
         * @param: workload_key, 任务key
         * @return: 惩罚分数, 负分
         */
        float getPenaltyScore(const string& workload_key);

    public:
    
        // 初始化m_regular_data, m_longterm_data
        bool initData();

        bool parseWorkloadKey(const string& workload_key, string& source, string& crawl_day);

        bool connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db);
        
        /*
         * 读取特定类别任务数据，分别写入m_regular_data和m_longterm_data
         * @param: type, 类别
         */
        bool readData(Type type, const string& host, const string& user, const string& passwd, const string& db);
        
        /*
         * 根据任务时间判断该任务是否为长期任务
         * @param: workload_key, 任务key
         * @param: type, 类别，不同的类别workload_key解析方式不同
         * @return: 0代表长期，1代表短期
         */
        int isLongtermTask(const string& workload_key, Type type);

        // 判断该时间槽是否应该爬取长期任务
        bool isLongtermTimeslot(int timeslot);

        /*
         *  不分类别，计算长期任务的得分
         *  @param: workload_key, 长期任务key
         *  @param: updatetime, 该任务上次更新时间
         *  @return: 该长期任务的得分
         */
        float getLongtermTaskScore(const string& workload_key, const string& updatetime);
        
        bool loadTaskError(const string& host, const string& user, const string& passwd, const string& db);

        /*
         * 反馈任务执行情况，对于不可抓的任务降权
         * @param: workload_key, 任务key
         * @return: 该任务的得分，负分
         */
        float feedbackTaskError(const string& workload_key);

        bool loadAssignedTasks(const string& host, const string& user, const string& passwd, const string& db);
        
        /*
         * 过滤上次分配的任务
         * @param: workload_key
         * @return: 真代表上次已分配，过滤
         */
        bool filterAssignedTask(const string& workload_key);

        bool readExpTaskKeys(vector<string>& tasks, const string& host, const string& user, const string& passwd, const string& db);
        bool readTmpTaskKeys(vector<string>& tasks, const string& sql, const string& host, const string& user, const string& passwd, const string& db);
        bool readTmpTasks(vector<string>& tasks, const string& sql, const string& host, const string& user, const string& passwd, const string& db);

    public:
        DATA m_regular_data;                                        // 例行任务数据
        DATA m_longterm_data;                                       // 长期任务数据
        TASK m_regular_tasks;                                       // 例行任务得分
        TASK m_longterm_tasks;                                      // 长期任务得分
        tr1::unordered_map<string, string> m_key_to_content;        // 映射任务key到内容的字典
        tr1::unordered_map<string, string> m_not_crawl_tasks;       // 不能爬取的任务
        tr1::unordered_map<string, int> m_assigned_tasks;           // 上次已分配的任务key

        BaseGenerator* m_generator;
        Valve* m_valve;

        DateTime m_today;
        DateTime m_now;
        
        int m_read_num;                                             // 记录第几次读取数据
        pthread_mutex_t m_locker;
};


#endif  // _TASK_GENERATOR_HPP
