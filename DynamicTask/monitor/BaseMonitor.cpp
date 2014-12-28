#include "BaseMonitor.hpp"

BaseMonitor::BaseMonitor()
{
    m_key_generator = new KeyGenerator();
}

BaseMonitor::~BaseMonitor()
{
    delete m_key_generator;
}

void BaseMonitor::init()
{
    if ( !createMonitorTable(HOST, "monitor", USR, PASSWD))
    {
        _ERROR("In BaseMonitor, cannot create monitor table!");
    }
}

bool BaseMonitor::createMonitorTable(const string& host, const string& db, const string& usr, const string& passwd)
{
    UseMysql mysql(host, db, usr, passwd);
    if ( !mysql.connect() )
    {
        return false;
    }

    //数据库准备就绪
    //建立train_task_monitor表
    ostringstream oss;
    oss << "create table if not exists train_task_monitor (id int unsigned not null auto_increment primary key, workload_key varchar(128) NOT NULL UNIQUE, source varchar(24), last_updatetime varchar(64), "
        << "last_price varchar(128), updatetime varchar(64), price varchar(128), price_wave varchar(24)) default charset=utf8;";
    string create_train_monitor_sql = oss.str();    //保存建表mysql指令
    if ( !mysql.query(create_train_monitor_sql) )
    {
        return false;
    }

    
    // 建flight_task_monitor
    ostringstream oss1;
    oss1 << "create table if not exists flight_task_monitor (id int unsigned not null auto_increment primary key, workload_key varchar(128) NOT NULL UNIQUE, source varchar(24), last_updatetime varchar(64), "
        << "last_price varchar(128), updatetime varchar(64), price varchar(128), price_wave varchar(24)) default charset=utf8;";
    string create_flight_monitor_sql = oss1.str();
    if ( !mysql.query(create_flight_monitor_sql))
    {
        return false;
    }
    // 建立room_task_monitor表
    ostringstream oss2;
    oss2 << "create table if not exists room_task_monitor (id int unsigned not null auto_increment primary key, workload_key varchar(128) NOT NULL UNIQUE, source varchar(24), last_updatetime varchar(64), last_price varchar(128), "
         << "updatetime varchar(64), price varchar(128), price_wave varchar(24)) default charset=utf8;";
    string create_room_monitor_sql = oss2.str();
    if ( !mysql.query(create_room_monitor_sql) )
    {
        return false;
    }
    

    return true;
}


string BaseMonitor::serializePrice(const Json::Value& price_value)
{
    Json::FastWriter jfw;
    string str = jfw.write(price_value);
    str = str.erase(str.rfind('\n'));
    return str;
}


//这里是将monitordata中的数据读取出来，然后分析新来的key是否存在，如果不存在则插入，如果存在则更新
//新来的key是通过crawldata里面分析出来的，这里的函数只是内层循环，每次过来一个key，都循环一次本函数
//注意，这里的monitordata是更新之前的数据，diffmonitordata是更新之后的数据，所以这里并没有修改
//monitordata
void BaseMonitor::insertMonitorData(const MONITOR_DATA& monitor_data, MONITOR_DATA& diff_monitor_data, const string& workload_key, const string& source, const string & updatetime, const string& price_str)
{
    //插入
    MONITOR_DATA::const_iterator it = monitor_data.find(workload_key);
    //这里的monitordata是一个map类型，第一个参数是workloadkey,第二个参数是一个vector
    //vector的内容为：workload_key, source, last_updatetime, last_price, updatetime, price, price_wave
    if (it != monitor_data.end())
    {
        //存在该key，更新
        vector<string> monitor_vec = it->second;
        monitor_vec[2] = monitor_vec[4];        //更新last_updatetime
        monitor_vec[3] = monitor_vec[5];        //更新last_price
        monitor_vec[4] = updatetime;            //更新updatetime
        monitor_vec[5] = price_str;             //更新price
        ostringstream oss;
        oss << getPriceWave(monitor_vec[3], monitor_vec[5]);    //通过上一次的价格和这一次的价格算出价格波动
        monitor_vec[6] = oss.str();             //更新价格变动
        diff_monitor_data[workload_key] = monitor_vec;         //更新monitor_data
    }
    else
    {
        //不存在该key，插入
        vector<string> monitor_vec;
        monitor_vec.push_back(workload_key);
        monitor_vec.push_back(source);
        monitor_vec.push_back("NULL");
        monitor_vec.push_back("NULL");
        monitor_vec.push_back(updatetime);
        monitor_vec.push_back(price_str);
        ostringstream oss;
        oss << getPriceWave("NULL", price_str);
        monitor_vec.push_back( oss.str() );
        diff_monitor_data[workload_key] = monitor_vec;
    }
}


//???这里不太懂，以后再回来看
float BaseMonitor::getPriceWave(const string& last_price_str, const string& price_str)
{
    //第一次更新价格，变动无限大
    if ("NULL" == last_price_str)
    {
        return 0.999;
    }

    //price解析
    Json::Value last_price_value, price_value;
    if ( !parsePrice(last_price_str, last_price_value) || !parsePrice(price_str, price_value) )
    {
        _ERROR("cannot parse price!");
        return 0;
    }

    float wave = 0.0;
    int num = 0;
    vector<string> last_price_members = last_price_value.getMemberNames();
    for (vector<string>::iterator it = last_price_members.begin(); it != last_price_members.end(); ++it, ++num)
    {
        string key = *it;
        float last_price_item = last_price_value.get(key, 0.0).asDouble();
        float price_item = price_value.get(key, 0.0).asDouble();
        wave += abs(last_price_item - price_item) / max(last_price_item, price_item);
    }
    wave = wave / num;
    return wave;
}


bool BaseMonitor::parsePrice(const string& price_str, Json::Value& price_value)
{
    Json::Reader reader;
    return reader.parse(price_str, price_value);
}


bool BaseMonitor::readMonitorData(MONITOR_DATA& monitor_data, const string& sql, int num_fields, const string& host, const string& db, const string& usr, const string& passwd)
{
    UseMysql mysql(host, db, usr, passwd);
    if (!mysql.connect())
    {
        return false;
    }
    if (!mysql.query(sql))
    {
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql.use_result();
        if (num_fields != mysql.num_fields(res))
        {
            _ERROR("IN readMonitorData, num_fields not match, read num_fields is %d", mysql_num_fields(res));
            return false;
        }
        MYSQL_ROW row;
        if (res)
        {
            while (row = mysql.fetch_row(res))
            {
                //注意，这里是把一个任务的workload_key当做map的key
                //key所对应的vec里面存放着该任务剩下的字段信息
                vector<string> vec;
                for (int i = 0; i < num_fields; ++i)
                {
                    vec.push_back(string(row[i]));
                }
                monitor_data[row[0]] = vec;
            }
        }
        mysql.free_result(res);
    }

    return true;
}


bool BaseMonitor::updateMonitorData(const MONITOR_DATA& diff_monitor_data, const string& host, const string& db, const string& usr, const string& passwd, string& sql)
{
    //判断是否有更新数据
    if (0 == diff_monitor_data.size())
    {
        _INFO("In updateMonitorData, no data update!");
        return false;
    }

    UseMysql mysql(host, db, usr, passwd);
    if (!mysql.connect())
        return false;

    ostringstream oss;
    //这里的monitordata是一个map类型，第一个参数是workloadkey,第二个参数是一个vector
    //vector的内容为：workload_key, source, last_updatetime, last_price, updatetime, price, price_wave
    for (MONITOR_DATA::const_iterator it = diff_monitor_data.begin(); it != diff_monitor_data.end(); ++it)
    {
        vector<string> monitor_vec = it->second;
        string workload_key = monitor_vec[0];
        string source = monitor_vec[1];
        string last_updatetime = monitor_vec[2];
        string last_price = monitor_vec[3];
        string updatetime = monitor_vec[4];
        string price = monitor_vec[5];
        string price_wave = monitor_vec[6];

        oss << "('" << workload_key << "','" << source << "','" << last_updatetime << "','" 
            << last_price << "','" << updatetime << "','" << price << "','" << price_wave << "'),";
    }
    sql += oss.str();
    sql.erase(sql.find_last_of(','), 1);
    //执行
    if (!mysql.query(sql))
    {
        return false;
    }

    return true;
}














