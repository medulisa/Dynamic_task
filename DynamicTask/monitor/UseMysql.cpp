#include "UseMysql.hpp"

UseMysql::UseMysql(const string& host, const string& db, const string& usr, const string& passwd)
    :host_(host), db_(db), usr_(usr), passwd_(passwd)
{
    mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
    mysql_init(mysql_);
}


UseMysql::~UseMysql()
{
    mysql_close(mysql_);
    delete mysql_;
}


bool UseMysql::connect()
{
    if (!mysql_real_connect(mysql_, host_.c_str(), usr_.c_str(), passwd_.c_str(), db_.c_str(), 0, NULL, 0))
    {
        _ERROR("[Connect to %s error: %s]", db_.c_str(), mysql_error(mysql_));
        return false;
    }

    if (mysql_set_character_set(mysql_, "utf8"))
    {
        _ERROR("[Set mysql characterset: %s]", mysql_error(mysql_));
        return false;
    }

    return true;
}


bool UseMysql::query(const string& sql)
{
    int t = mysql_query(mysql_, sql.c_str());
    if (t != 0)
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql_), sql.c_str());
        return false;
    }

    return true;
}

MYSQL_RES* UseMysql::use_result()
{
    return mysql_use_result(mysql_);
}

MYSQL_ROW UseMysql::fetch_row(MYSQL_RES* res)
{
    return mysql_fetch_row(res);
}

int UseMysql::num_fields(MYSQL_RES* res)
{
    return mysql_num_fields(res);
}


void UseMysql::free_result(MYSQL_RES* res)
{
   mysql_free_result(res);
}


