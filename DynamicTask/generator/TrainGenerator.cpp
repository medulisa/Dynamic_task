#include "TrainGenerator.hpp"

TrainGenerator* TrainGenerator::m_pInstance = NULL;

TrainGenerator::TrainGenerator()
{
}

TrainGenerator::TrainGenerator(DateTime now, DateTime today):m_now(now), m_today(today)
{
}

TrainGenerator::~TrainGenerator()
{
}

float TrainGenerator::getTaskScore(const string& workload_key, const string& updatetime, float price_wave)
{
    float score = 0.0;
    
    // 动态得分
    float update_score = getUpdateRewardScore(updatetime);

    // 最终得分
    score = update_score;

    return score;
}

string TrainGenerator::key2Content(const string& workload_key)
{
    vector<string> vec;
    SplitString(workload_key, "_", &vec);
    if( 4 != vec.size() )
    {
        _ERROR("[IN TrainGenerator::key2Content, wrong workload_key: %s]", workload_key);
        return "NULL";
    }
    string content;
    content = vec[0] + "&" + vec[1] + "&" + vec[3];
    return content;
}

float TrainGenerator::getUpdateRewardScore(const string& updatetime)
{
    // 更新时间越久，得分越高
    //
    if( "NULL" == updatetime )
        return 200.0;
    
    // 获取未更新时间（秒）
    DateTime update_dt  = DateTime::Parse(updatetime, "yyyy-MM-dd HH:mm:ss");
    TimeSpan ts = m_now - update_dt;
    int non_update_seconds = ts.GetTotalSeconds();
    
    if( non_update_seconds > 3*24*3600 )
        return 100.0;
    else
        return 1.0*non_update_seconds/(3*24*3600);
}

