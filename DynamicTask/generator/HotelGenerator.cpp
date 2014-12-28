#include <iostream>
#include "HotelGenerator.hpp"

HotelGenerator* HotelGenerator::m_pInstance = NULL;

HotelGenerator::HotelGenerator(DateTime now, DateTime today):m_now(now), m_today(today)
{
    loadHotelWorkload("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_hotel");
}


HotelGenerator::HotelGenerator()
{
    loadHotelWorkload("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_hotel");
}

HotelGenerator::~HotelGenerator()
{
}

// public
//
float HotelGenerator::getTaskScore(const string& workload_key, const string& updatetime, float price_wave)
{
    float score = 0.0;
    
    // 解析workload_key
    vector<string> vec;
    SplitString(workload_key, "|", &vec);
    string city = vec[0];
    string hotel_sourceid = vec[1];
    string source = vec[2];
    int days = atoi( vec[3].c_str() );
    string checkin_day = vec[4];
    string hotel_id = getHotelID(source, hotel_sourceid);

    // 动态得分
    float update_score = getUpdateRewardScore(updatetime);
    
    // 静态日期得分
    float checkin_score = getCheckinDayScore(checkin_day);
    float interval_score = getDayIntervalScore(checkin_day);
    float days_score = getDaysScore(days);

    // 静态酒店得分
    float city_score = getCityScore(city);
    float hotel_score = getHotelScore(hotel_id);

    float source_score = getSourceScore(source);

    // 最终得分
    //score = price_wave + 2.0*update_score + checkin_score*interval_score*days_score + city_score*hotel_score + source_score;
    score = update_score;
    return score;
}

string HotelGenerator::key2Content(const string& workload_key)
{
    vector<string> vec;
    SplitString(workload_key, "|", &vec);
    if(vec.size() != 5)
    {
        _ERROR("[IN HotelGenerator::key2Content, wrong workload_key: %s]", workload_key.c_str());
        return "NULL";
    }
    string key = vec[0] + "|" + vec[1] + "|" + vec[2];
    string content;
    if( m_key_to_content.find(key) != m_key_to_content.end() )
        content =  m_key_to_content[key];
    else
    {
        //_INFO("hotel key, %s", workload_key.c_str());
        return "NULL";
    }
    //if(vec[2] == "elongHotel" || vec[2] == "hotelsHotel")
    if( content.find("&&") != string::npos )
    {
        content += vec[3] + "&&" + vec[4];
    }
    else
    {
        content += vec[3] + "&" + vec[4];
    }
    return content;
}


float HotelGenerator::getCityScore(const string& city_en)
{
    if(city_en == "NULL")
        return 1.0;
    else
        return m_city_level_map[city_en];
}

float HotelGenerator::getHotelScore(const string& uid)
{
    return 1.0;
}

float HotelGenerator::getCheckinDayScore(const string& checkin_day)
{
    return 1.0;
}

float HotelGenerator::getDayIntervalScore(const string& checkin_day)
{
    // 获取间隔天数
    DateTime checkin_dt = DateTime::Parse(checkin_day, "yyyyMMdd");
    TimeSpan ts = checkin_dt - m_today;
    int interval_days = ts.GetDays();
    // Interval越小，得分越高
    return (100.0 - interval_days) / 100.0;
}

float HotelGenerator::getDaysScore(int days)
{
    return 1.0;
}

float HotelGenerator::getSourceScore(const string& source)
{
    return getSourceLevelScore(source) * getSourceRewardScore(source);
}


float HotelGenerator::getUpdateRewardScore(const string& updatetime)
{
    // 更新时间越久，得分越高
    //
    if(updatetime == "NULL")
        return 200.0;
    
    // 获取未更新时间（秒）
    DateTime update_dt  = DateTime::Parse(updatetime, "yyyy-MM-dd HH:mm:ss");
    TimeSpan ts = m_now - update_dt;
    int non_update_seconds = ts.GetTotalSeconds();
    
    // 如果三天未更新，返回一个极大值
    if(non_update_seconds > 3*24*3600)
        return 100.0;
    else
        return 1.0*non_update_seconds/(3*24*3600);
}

// private
//
//
string HotelGenerator::getHotelID(const string& source, const string& hotel_sourceid)
{
    // 针对以城市爬取的源: agoda,biyi
    if(hotel_sourceid == "NULL")
        return "NULL";
    
    string key = source + "&" + hotel_sourceid;
    if(m_hotel_id_map.find(key) != m_hotel_id_map.end())
        return m_hotel_id_map[key];

    return "NULL";
}


bool HotelGenerator::connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db)
{
    mysql_init(mysql);
    if (!mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 0, NULL, 0)) 
    {   
        _ERROR("[Connect to %s error: %s]", db.c_str(), mysql_error(mysql));
        return false;
    }   
    // 设置字符编码
    if (mysql_set_character_set(mysql, "utf8"))
    {
        _ERROR("[Set mysql characterset: %s]", mysql_error(mysql));
        return false;
    }
    return true;
}


bool HotelGenerator::loadCityLevel(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadCityLevel, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT city, grade FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadCityLevel, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string city = row[0];
                float grade = atof(row[1]);
                m_city_level_map[city] = grade;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool HotelGenerator::loadHotelID(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadHotelID, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT source, sid, uid FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadHotelID, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string source = string(row[0]) + "Hotel";
                string sid = row[1];
                string uid = row[2];
                string key = source + "&" + sid;
                m_hotel_id_map[key] = uid;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool HotelGenerator::loadHotelStar(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadHotelStar, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT uid, star FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadHotelStar, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string uid = row[0];
                float star = atof(row[1]);
                m_hotel_star_map[uid] = star;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool HotelGenerator::loadHotelComment(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadHotelComment, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT distinct uid, count(*) FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadHotelComment, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string uid = row[0];
                float comment_num = atof(row[1]);
                m_hotel_comment_map[uid] = comment_num;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool HotelGenerator::loadHotelGrade(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadHotelGrade, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT uid, grade FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadHotelGrade, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string uid = row[0];
                float grade = atof(row[1]);
                m_hotel_grade_map[uid] = grade;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool HotelGenerator::loadHotelWorkload(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In HotelGenerator::loadHotelWorkload, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT workload_key, content FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN HotelGenerator::loadHotelWorkload, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    else
    {   
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if(res)
        {
            while( row = mysql_fetch_row(res) )
            {
                string key = row[0];
                string content = row[1];
                m_key_to_content[key] = content;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}

float HotelGenerator::getSourceLevelScore(const string& source)
{
    return 1.0;
}

float HotelGenerator::getSourceRewardScore(const string& source)
{
    if(source == "youzhanHotel")
    {
        return -10.0;
    }
    return 1.0;
}
