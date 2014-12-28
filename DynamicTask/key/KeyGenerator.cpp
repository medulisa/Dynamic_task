#include <sstream>
#include <fstream>
#include <iostream>
#include "common/service_log.hpp"
#include "KeyGenerator.hpp"

KeyGenerator::KeyGenerator()
{
    if( !loadFlightSource() || !loadCity() )
    {
        _ERROR("IN keyGenerator: load data error!");
    }
}

KeyGenerator::~KeyGenerator()
{
}


bool KeyGenerator::connect2DB(MYSQL* mysql, const string& host, const string& db, const string& user, const string& passwd)
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


bool KeyGenerator::loadFlightSource()
{
    // 连接数据库
    string host = "10.66.115.222";
    string db = "workload";
    string user = "reader";
    string passwd = "miaoji1109";
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, db, user, passwd) )
    {
        return false;
    }
    // 查询源的信息
    string sql = "SELECT source_name, workload_rule, workload_spliter, workload_type, source_status, source_class, source FROM workload_flight_source";
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
                source_map["source"] = row[6];
                m_flight_source[ row[6] ] = source_map;
            }
        }
        mysql_free_result(res);
    }

    mysql_close(mysql);
    delete mysql;
    _INFO("IN KeyGenrator, load Flight Source successfully!");
    return true;
}


bool KeyGenerator::loadCity()
{
    // 连接数据库
    string host = "10.66.115.222";
    string db = "basic";
    string user = "reader";
    string passwd = "miaoji1109";
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, db, user, passwd) )
    {
        return false;
    }
    // 查询城市信息
    string sql = "SELECT name, name_en, py, tri_code FROM city_all";
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
                tr1::unordered_map<string, string> city_map;
                city_map["city_name"] = row[0];
                city_map["city_en"] = row[1];
                city_map["city_pinyin"] = row[2];
                city_map["tri_code"] = row[3];
                m_city[ row[0] ] = city_map;
            }
        }
        mysql_free_result(res);
    }

    mysql_close(mysql);
    delete mysql;
    _INFO("IN KeyGenrator, load City successfully!");
    return true;
}


string KeyGenerator::getFlightOnewayKey(const string& source, const string& dept_id, const string& dest_id, const string& dept_day)
{
    /*  根据rule生成workload_content
     *
    string rule = m_flight_source[source]["rule"];
    string spliter = m_flight_source[source]["spliter"];
    string type = m_flight_source[source]["type"];
    string workload_key = getKeyFromRule(rule, spliter, type, dept_id, dest_id);
    workload_key += dept_day;
    */
    string workload_key = dept_id + "_" + dest_id + "_" + m_flight_source[source]["name"] + "_" + dept_day;
    return workload_key;
}

string KeyGenerator::getFlightRoundKey(const string& source, const string& dept_id, const string& dest_id, const string& dept_day, const string& dest_day)
{
    /*
    string rule = m_flight_source[source]["rule"];
    string spliter = m_flight_source[source]["spliter"];
    string type = m_flight_source[source]["type"];
    string workload_key = getKeyFromRule(rule, spliter, type, dept_id, dest_id);
    workload_key += dept_day;
    workload_key += spliter + dest_day;
    */
    string workload_key = dept_id + "_" + dest_id + "_" + m_flight_source[source]["name"] + "_" + dept_day + "_" + dest_day;
    return workload_key;
}

string KeyGenerator::getRoomKey(const string& source, const string& city_name, const string& hotel_id, const string& checkin_day, const string& checkout_day)
{
    // 城市英文名
    string city_en = m_city[city_name]["city_en"];
    // 计算入住日期
    time_t checkin_seconds = DateTime::Parse(checkin_day, "yyyyMMdd").GetSecondsSinceEpoch();
    time_t checkout_seconds = DateTime::Parse(checkout_day, "yyyyMMdd").GetSecondsSinceEpoch();
    int days = (int(checkout_seconds) - int(checkin_seconds)) / 86400;
    
    // 按酒店规则生成workload_key
    if(source == "biyi" || source == "agoda")
    {
        ostringstream oss1;
        oss1 << city_en << "|" << "NULL" << "|" << source << "Hotel|" << days << "|" << checkin_day;
        return oss1.str();
    }
    else
    {
        ostringstream oss2;
        oss2 << "NULL" << "|" << hotel_id << "|" << source << "Hotel|" << days << "|" << checkin_day;
        return oss2.str();
    }
}

string KeyGenerator::getTrainKey(const string& source, const string& dept_city, const string& dest_city, const string& dept_day)
{
    string workload_key = m_city[dept_city]["tri_code"] + "_" + m_city[dest_city]["tri_code"] + "_" + source + "Rail" + "_" + dept_day;
    //TODO
    return workload_key;
}







