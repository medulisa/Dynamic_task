#ifndef _USEMYSQL_HPP_
#define _USEMYSQL_HPP_ 

#include <string>
#include <mysql/mysql.h>
#include "common/service_log.hpp"
#include "CommonFuc.hpp"

using namespace std;

class UseMysql
{
    public:
        UseMysql(const string& host, const string& db, const string& usr, const string& passwd);
        ~UseMysql();

        bool connect();
        bool query(const string& sql);
        MYSQL_RES* use_result();
        MYSQL_ROW fetch_row(MYSQL_RES* res);
        int num_fields(MYSQL_RES* res);
        void free_result(MYSQL_RES* res);
    private:
        MYSQL* mysql_;
        string host_;
        string db_;
        string usr_;
        string passwd_;
        
};

#endif  /*_USEMYSQL_HPP_*/
