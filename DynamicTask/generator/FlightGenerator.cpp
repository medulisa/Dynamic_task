#include <sstream>
#include "FlightGenerator.hpp"

FlightGenerator* FlightGenerator::m_pInstance = NULL;

FlightGenerator::FlightGenerator()
{
    loadAirport("10.66.115.222", "root", "miaoji@2014!", "basic", "airport");
    loadCity("10.66.115.212", "reader", "miaoji1109", "onlinedb", "city");
    loadFlightSource("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_flight_source");
    loadFlightPair("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_flight_pair");
}

FlightGenerator::FlightGenerator(DateTime now, DateTime today):m_now(now), m_today(today)
{
    loadAirport("10.66.115.222", "root", "miaoji@2014!", "basic", "airport");
    loadCity("10.66.115.212", "reader", "miaoji1109", "onlinedb", "city");
    loadFlightSource("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_flight_source");
    loadFlightPair("10.66.115.222", "root", "miaoji@2014!", "workload", "workload_flight_pair");
}


FlightGenerator::~FlightGenerator()
{
}

float FlightGenerator::getTaskScore(const string& workload_key, const string& updatetime, float price_wave)
{
    float score = 0.0;
    
    // 解析workload_key
    vector<string> vec;
    SplitString(workload_key, "_", &vec);
    string dept_id = vec[0];
    string dest_id = vec[1];
    string source= vec[2];
    string dept_day = vec[3];

    // 动态得分
    float update_score = getUpdateRewardScore(updatetime);

    // 源得分
    float source_score = getSourceScore(source);
/*
    // 城市得分
    float city_score = 0.0;
    string dept_city_cn, dest_city_cn;
    if( m_airport_map.find(dept_id) == m_airport_map.end() || m_airport_map.find(dest_id) == m_airport_map.end() )
        city_score = 0.0;
    city_score = getCityScore( m_airport_map[dept_id]["city_cn"], m_airport_map[dest_id]["city_cn"] );
*/  
    // 总得分
    //score = price_wave + 2*update_score + source_score + city_score;
    score = update_score + source_score;
    
    return score;
}

string FlightGenerator::key2Content(const string& key)
{
    vector<string> vec;
    SplitString(key, "_", &vec);
    string source, dept_id, dest_id, dept_day, dest_day, rule, type, spliter;
    string content = "NULL";
    if(vec.size() < 4)
    {
        _ERROR("[IN FlightGenerator::key2Content, wrong workload_key: %s ]", key.c_str());
        return content;
    }
    if(vec.size() == 4 && vec[2].find("Flight") != string::npos)
    {
        dept_id = vec[0];
        dest_id = vec[1];
        source = vec[2];
        dept_day = vec[3];
        rule = m_flight_source[source]["rule"];
        spliter = m_flight_source[source]["spliter"];
        type = m_flight_source[source]["type"];
        content = getContentFromRule(rule, spliter, type, dept_id, dest_id);
        content += dept_day;
    }
    else if(vec.size() == 5 && vec[2].find("RoundFlight") != string::npos)
    {
        dept_id = vec[0];
        dest_id = vec[1];
        source = vec[2];
        dept_day = vec[3];
        dest_day = vec[4];
        rule = m_flight_source[source]["rule"];
        spliter = m_flight_source[source]["spliter"];
        type = m_flight_source[source]["type"];
        content = getContentFromRule(rule, spliter, type, dept_id, dest_id);
        content += dept_day;
        content += spliter + dest_day;
    }
    return content;
}


float FlightGenerator::getCityScore(const string& dept_city, const string& dest_city)
{
    if( m_city_map.find(dept_city) == m_city_map.end() || m_city_map.find(dest_city) == m_city_map.end() )
        return 1.0;
    
    string dept_city_grade, dest_city_grade;
    dept_city_grade = m_city_map[dept_city]["grade"];
    dest_city_grade = m_city_map[dest_city]["grade"];
    int grade = atoi( dept_city_grade.c_str() ) + atoi( dest_city_grade.c_str() );
    
    switch(grade)
    {
        case 2:
            return 1.0;
        case 3:
            return 0.5;
        case 4:
            return 0.3;
        case 5:
            return 0.1;
        default:
            return 0.0;
    }
}

float FlightGenerator::getAirportScore(const string& dept_id, const string& dest_id)
{
    return 1.0;
}

float FlightGenerator::getSourceScore(const string& source)
{
    if( "airtickets" == source)
    {
        return 1.0;
    }
    return 0.0;
}

float FlightGenerator::getDayScore(const string& dept_day)
{
    return 1.0;
}


float FlightGenerator::getDayIntervalScore(const string& dept_day)
{
    return 1.0;
}

float FlightGenerator::getUpdateRewardScore(const string& updatetime)
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
    {
        //_INFO("[hi, non_update_seconds is %d]", non_update_seconds);
        return 100.0;
    }
    else
    {
        //_INFO("[update score is %f]", 2.0*non_update_seconds/(3*24*3600) );
        return 1.0*non_update_seconds/(3*24*3600);
    }
}


bool FlightGenerator::connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db)
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

bool FlightGenerator::loadAirport(const string&host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In FlightGenerator::loadAirport, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT iatacode, city, city_en_name, city_cn_name, city_pinyin, class FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN FlightGenerator::loadAirport, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
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
                tr1::unordered_map<string, string> airport_map;
                airport_map["airport"] = row[0];
                airport_map["city"] = row[1];
                airport_map["city_en"] = row[2];
                airport_map["city_cn"] = row[3];
                airport_map["city_pinyin"] = row[4];
                airport_map["class"] = row[5];
                m_airport_map[row[0]] = airport_map;
            }
        }
        mysql_free_result(res);
    }
    _INFO("[ IN FlightGenerator::loadAirport, load airport successfully!]");
    mysql_close(mysql);
    delete mysql;
    return true;
}

bool FlightGenerator::loadCity(const string&host, const string& user, const string& passwd, const string& db, const string& table)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        _ERROR("[In FlightGenerator::loadCity, Cannot connect to db!]");
        return false;
    }
    string sql = "SELECT name, name_en, py, grade FROM " + table + "where status = 'Open'";
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {   
        _ERROR("[IN FlightGenerator::loadCity, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
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
                tr1::unordered_map<string, string> city_map;
                city_map["city_name"] = row[0];
                city_map["city_en"] = row[1];
                city_map["city_pinyin"] = row[2];
                city_map["grade"] = row[3];
                m_city_map[row[0]] = city_map;
            }
        }
        mysql_free_result(res);
    }
    _INFO("[ IN FlightGenerator::loadCity, load city successfully!]");
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool FlightGenerator::loadFlightSource(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    // 连接数据库
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        return false;
    }
    // 查询源的信息
    string sql = "SELECT source_name, workload_rule, workload_spliter, workload_type, source_status, source_class FROM " + table;
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
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
                tr1::unordered_map<string, string> source_map;
                source_map["name"] = row[0];
                source_map["rule"] = row[1];
                source_map["spliter"] = row[2];
                source_map["type"] = row[3];
                source_map["status"] = row[4];
                source_map["class"] = row[5];
                m_flight_source[ row[0] ] = source_map;
            }
        }
        mysql_free_result(res);
    }

    mysql_close(mysql);
    delete mysql;
    _INFO("IN FlightGenerator, load Flight Source successfully!");
    return true;
}


bool FlightGenerator::loadFlightPair(const string& host, const string& user, const string& passwd, const string& db, const string& table)
{
    // 连接数据库
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {
        return false;
    }
    // 查询航班航线信息
    for(FLIGHT_SOURCE::iterator it = m_flight_source.begin(); it != m_flight_source.end(); ++it)
    {
        vector< pair<string, string> > pair_vec;
        string source_name = it->first;
        ostringstream oss;
        oss << "SELECT dept_airport, dest_airport FROM " << table << " WHERE " << source_name << "=1";
        if(int t = mysql_query(mysql, oss.str().c_str()) != 0)
        {
            _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), oss.str().c_str());
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
                    string dept_airport = row[0];
                    string dest_airport = row[1];
                    pair_vec.push_back( pair<string, string>(dept_airport, dest_airport) );
                    m_flight_pair[source_name] = pair_vec;
                }
            }
            mysql_free_result(res);
        }
    }
    mysql_close(mysql);
    delete mysql;
    _INFO("IN FlightGenerator, load Flight Pair successfully!");
    return true;
}


string FlightGenerator::getContentFromRule(const string& rule, const string& spliter, const string& type, const string& dept_id, const string& dest_id)
{
    if(type != "oneway" && type != "round")
    {
        _ERROR("IN FlightGenerator::getContentFromRule, wrong type! TYPE: %s", type.c_str());
        return "";
    }

    string result = "";

    // 分割rule
    vector<string> keys;
    SplitString(rule, "+", &keys);
    // 去除rule中的日期
    (keys).pop_back();
    if(type == "round")
    {
        (keys).pop_back();
    }

    for(vector<string>::iterator it = (keys).begin(); it != (keys).end(); ++it)
    {
        string key = *it;
        if( key[key.length()-1] == '1' )
        {
            key = key.substr(0, key.length()-1);
            result += m_airport_map[dept_id][key] + spliter;
        }
        else if( key[key.length()-1] == '2' )
        {
            key = key.substr(0, key.length()-1);
            result += m_airport_map[dest_id][key] + spliter;
        }
        else
        {
            _ERROR("wrong rule!");
        }
    }
    return result;
}

