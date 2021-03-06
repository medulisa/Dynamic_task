#include <iostream>
#include <sstream>
#include "TaskGenerator.hpp"
using namespace std;



TaskGenerator::TaskGenerator()
{
    m_now = DateTime::Now();
    m_today = DateTime::Today();
    m_read_num = 0;

    initData();
    readData(Flight, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    //readData(Hotel, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    //readData(Train, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    m_read_num += 1;
    loadTaskError("10.66.115.222", "root", "miaoji@2014!", "monitor");
    _INFO("load task error ok");
    loadAssignedTasks("10.66.115.222", "root", "miaoji@2014!", "workload");
    _INFO("[last assigned task size is %d]", m_assigned_tasks.size());
    _INFO("[error task size is %d]", m_not_crawl_tasks.size());
    _INFO("[m_regular_data size is %d, m_longterm_data size is %d]", m_regular_data[Flight].size()+m_regular_data[Hotel].size(), m_longterm_data[Flight].size()+m_longterm_data[Hotel].size() );
    
    m_valve = Valve::getInstance();

    m_locker = PTHREAD_MUTEX_INITIALIZER;
}


TaskGenerator::~TaskGenerator()
{
}


bool TaskGenerator::writeTask2DB(const string& host, const string& user, const string& passwd, const string& db, bool is_del)
{
    m_now = DateTime::Now();
    
    // 读取监控数据
    readData(Flight, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    readData(Hotel, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    readData(Train, "10.66.115.222", "root", "miaoji@2014!", "monitor");
    m_read_num += 1;   
    
    // 生成任务
    int hour = m_now.GetHour();
    int minute = m_now.GetMinute();
    
    // 从下个时间槽开始插入
    int timeslot = (hour*60+minute) / INTERVAL + 2;
    _INFO("before assign");
    assignTaskByTimeslot(timeslot);
    _INFO("after assign");
    _INFO("before tmp, task size is %d", m_regular_tasks.size());
    assignTmpTask(timeslot);
    _INFO("after tmp, task size is %d", m_regular_tasks.size());
    
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        return false;
    }

    // 写库之前先把之前生成的任务删除
    _INFO("delete longterm");
    if( is_del )
    {
        ostringstream del_oss;
        del_oss << "DELETE FROM workload_longterm where timeslot = " << timeslot << " or timeslot = " << timeslot+1;
        string del_sql = del_oss.str();
        if(int t = mysql_query(mysql, del_sql.c_str()) != 0)
        {
            _ERROR("[In TaskGenerator::writeTask2DB, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), del_sql.c_str());
        }
            
    }
    // 生成sql语句
    ostringstream oss;
    oss << "INSERT INTO " << "workload_longterm" << " (workload_key, content, source, crawl_day, score, timeslot) VALUES ";
    if( 0 == m_longterm_tasks.size() )
    {
        _INFO("write 0 tasks to db!");
        return false;
    }
    srand(time(NULL));
    _INFO("[longterm task size is %d]", m_longterm_tasks.size());
    for(TASK::const_iterator it = m_longterm_tasks.begin(); it != m_longterm_tasks.end(); ++it)
    {
        string workload_key = it->first;
        if( m_key_to_content.find(workload_key) == m_key_to_content.end() )
        {
            //_INFO("[LONGTERM, no key!]");
            continue;
        }
        string workload_content = m_key_to_content[workload_key];
        if( workload_content == "NULL" )
        {
            //_INFO("[LONGTERM, no content]");
            continue;
        }
        if( workload_content.find("'") != string::npos )
            continue;
        string source = "", crawl_day = "";
        parseWorkloadKey(workload_key, source, crawl_day);
        float score = it->second;
        int i = rand() % 2;
        oss << "('" << workload_key << "','" << workload_content << "','" << source << "','" << crawl_day << "','" << score << "','" << (timeslot+i)%288 << "'),";
    }
    string sql = oss.str();
    sql.erase(sql.find_last_of(','), 1);
    // 写库
    _INFO("before write longterm");
    if(int t = mysql_query(mysql, sql.c_str()) != 0)
    {
        _ERROR("[In TaskGenerator::writeTask2DB, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        return false;
    }
    _INFO("after write longterm");
    //
    
    if(!isLongtermTimeslot(timeslot))
    {
        // 写库之前先把之前生成的任务删除
        if( is_del )
        {
            ostringstream del_oss;
            del_oss << "DELETE FROM workload_" << m_today.ToString("yyyyMMdd") << " where timeslot = " << timeslot << " or timeslot = " << timeslot+1;
            string del_sql = del_oss.str();
            if(int t = mysql_query(mysql, del_sql.c_str()) != 0)
            {
                _ERROR("[In TaskGenerator::writeTask2DB, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), del_sql.c_str());
            }
        }

        bool cross_day = false;              // 标记是否跨天
        // 生成sql语句
        ostringstream oss1, oss2;
        string table1 = "workload_" + m_today.ToString("yyyyMMdd");
        oss1 << "INSERT INTO " << table1 << " (workload_key, content, source, score, timeslot) VALUES ";
        string table2 = "workload_" + (m_today+TimeSpan(1,0,0,0)).ToString("yyyyMMdd");
        oss2 << "INSERT INTO " << table2 << " (workload_key, content, source, score, timeslot) VALUES ";
        _INFO("[regular task size is %d]", m_regular_tasks.size());
        for(TASK::const_iterator it = m_regular_tasks.begin(); it != m_regular_tasks.end(); ++it)
        {
            string workload_key = it->first;
            if( m_key_to_content.find(workload_key) == m_key_to_content.end() )
            {
                //_INFO("[REGULAR, no key!]");
                continue;
            }
            string workload_content = m_key_to_content[workload_key];
            if( workload_content == "NULL" )
            {
                //_INFO("[REGULAR, no content]");
                continue;
            }
            if( workload_content.find("'") != string::npos )
            {
                //_INFO("[REGULAR, has fenhao]");
                continue;
            }
            string source = "", crawl_day = "";
            parseWorkloadKey(workload_key, source, crawl_day);
            float score = it->second;
            int i = rand() % 2;
            if(timeslot+i > 287)
            {
                cross_day = true;
                oss2 << "('" << workload_key << "','" << workload_content << "','" << source << "','" << score << "','" << (timeslot+i)%288 << "'),";
            }
            oss1 << "('" << workload_key << "','" << workload_content << "','" << source << "','" << score << "','" << (timeslot+i)%288 << "'),";
        }
        string sql1 = oss1.str();
        sql1.erase(sql1.find_last_of(','), 1);
        // 写库
        if(int t = mysql_query(mysql, sql1.c_str()) != 0)
        {
            _ERROR("[In TaskGenerator::writeTask2DB, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql1.c_str());
            return false;
        }
        if(cross_day)
        {
            string sql2 = oss2.str();
            sql2.erase(sql2.find_last_of(','), 1);
            if(int t = mysql_query(mysql, sql2.c_str()) != 0)
            {
                _ERROR("[In TaskGenerator::writeTask2DB, mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql2.c_str());
                return false;
            }
        }
    }
    
    //释放task
    TASK* p = new TASK;
    m_longterm_tasks.swap(*p);
    delete p;
    TASK* p1 = new TASK;
    m_regular_tasks.swap(*p1);
    delete p1;

    mysql_close(mysql);
    delete mysql;
    return true;

}

bool TaskGenerator::assignTaskByTimeslot(int timeslot)
{
    if( isLongtermTimeslot(timeslot) ) 
    {
        // 分配长期任务并释放内存
        //getLongtermTasks(m_longterm_data[Flight], m_longterm_tasks, Flight, Longterm);
        //getLongtermTasks(m_longterm_data[Hotel], m_longterm_tasks, Hotel, Longterm);
        //getLongtermTasks(m_longterm_data[Train], m_longterm_tasks, Train, Longterm);
    }
    else
    {
        // 分配例行和长期任务
        //getLongtermTasks(m_longterm_data[Flight], m_longterm_tasks, Flight, Longterm);
        //getLongtermTasks(m_longterm_data[Hotel], m_longterm_tasks, Hotel, Longterm);
        //getLongtermTasks(m_longterm_data[Train], m_longterm_tasks, Train, Longterm);
        TASK in_tasks;
        getTasks(m_regular_data[Flight], in_tasks, m_regular_tasks, Flight, Regular);
        //getTasks(m_regular_data[Hotel], in_tasks, m_regular_tasks, Hotel, Regular);
        //getTasks(m_regular_data[Train], in_tasks, m_regular_tasks, Train, Regular);
    }
    return true;
}


bool TaskGenerator::assignTmpTask(int timeslot)
{
    TASK* task_data;
    if( isLongtermTimeslot(timeslot) )
    {
        task_data = &m_longterm_tasks;
    }
    else
    {
        task_data = &m_regular_tasks;
    }

    vector<string> task_keys;
    
    // 读取验证任务key
    if( !readExpTaskKeys(task_keys, "10.66.115.222", "root", "miaoji@2014!", "workload") )
    {
        return false;
    }
    
    /*
    int num = task_keys.size();
    // 读取临时火车任务
    _INFO("before read Rail task!");  
    srand(time(NULL));
    int i = 30 + rand()%40;
    string day = (m_today+TimeSpan(i, 0, 0, 0)).ToString("yyyyMMdd");
    string train_sql = "SELECT workload_key, content FROM workload_longterm where source = 'europerailRail' and crawl_day='" + day + "'" + " and timeslot = -1";
    string train_sql1 = "SELECT workload_key, content FROM workload_longterm where source = 'voyagesRail' and crawl_day='" + day + "'" + " and timeslot = -1";
    if( !readTmpTasks(task_keys, train_sql, "10.66.115.222", "root", "miaoji@2014!", "workload") || !readTmpTasks(task_keys, train_sql1, "10.66.115.222", "root", "miaoji@2014!", "workload") )
    //if( !readTmpTasks(task_keys, train_sql, "10.66.115.222", "root", "miaoji@2014!", "workload") )
    {
        return false;
    }
    _INFO("[Rail size is %d]", task_keys.size()-num );
    */
    // 读取临时ctripHotel任务key
    _INFO("before read Ctrip task");
    string ctrip_sql1 = "SELECT workload_key FROM workload_" + m_today.ToString("yyyyMMdd") + " WHERE source = 'ctripHotel'" + " and timeslot = -1";
    string ctrip_sql2 = string("SELECT workload_key FROM workload_longterm WHERE source = 'ctripHotel'") + " and timeslot = -1";
    if( !readTmpTaskKeys(task_keys, ctrip_sql1, "10.66.115.222", "root", "miaoji@2014!", "workload") )
    {
        return false;
    }
    if( !readTmpTaskKeys(task_keys, ctrip_sql2, "10.66.115.222", "root", "miaoji@2014!", "workload") )
    {
        return false;
    }
    _INFO("after read Ctrip task");

    for(int i = 0; i < task_keys.size(); i++)
    {
        string workload_key = task_keys[i];
        task_data->insert(make_pair(workload_key, -1.0));
        if(workload_key.find("Flight") != string::npos)
        {
            m_generator = FlightGenerator::getInstance(m_now, m_today);
        }
        else if(workload_key.find("Hotel") != string::npos)
        {
            m_generator = HotelGenerator::getInstance(m_now, m_today);
        }
        else
        {
            continue;
        }
        string content = m_generator->key2Content(workload_key);
        m_key_to_content.insert(make_pair(workload_key, content));
    }
    _INFO("[In tmp, task data size is %d]", task_data->size());
}

bool TaskGenerator::getTasks(const TYPE_DATA& type_data, TASK in_tasks, TASK& out_tasks, Type type, Mode mode)
{
    string js_str = "{\"items\":[{\"name\":\"airberlinFlight\", \"ub\": 300, \"lb\": 5},{\"name\":\"biyiHotel\", \"ub\": 500, \"lb\": 5},{\"name\":\"bookingHotel\", \"ub\": 1000, \"lb\": 5},{\"name\":\"csairFlight\", \"ub\": 200, \"lb\": 5},{\"name\":\"ctripFlight\", \"ub\": 1500, \"lb\": 5},{\"name\":\"ctripHotel\", \"ub\": 100, \"lb\": 5},{\"name\":\"easyjetFlight\", \"ub\": 500, \"lb\": 5},{\"name\":\"ebookersFlight\", \"ub\": 1500, \"lb\": 5},{\"name\":\"elongFlight\", \"ub\": 500, \"lb\": 5},{\"name\":\"elongHotel\", \"ub\": 1500, \"lb\": 5},{\"name\":\"europerailRail\", \"ub\": 150, \"lb\": 5},{\"name\":\"expediaFlight\", \"ub\": 1500, \"lb\": 5},{\"name\":\"hotelclubHotel\", \"ub\": 1000, \"lb\": 5},{\"name\":\"hotelsHotel\", \"ub\": 1500, \"lb\": 5},{\"name\":\"jijitongFlight\", \"ub\": 1000, \"lb\": 5},{\"name\":\"jijitongRoundFlight\", \"ub\": 5, \"lb\": 5},{\"name\":\"ryanairFlight\", \"ub\": 200, \"lb\": 5},{\"name\":\"tongchengFlight\", \"ub\": 1000, \"lb\": 5},{\"name\":\"tripstaFlight\", \"ub\": 300, \"lb\": 5},{\"name\":\"ufeifanFlight\", \"ub\": 300, \"lb\": 5},{\"name\":\"venereHotel\", \"ub\": 1000, \"lb\": 5},{\"name\":\"vuelingFlight\", \"ub\": 500, \"lb\":5},{\"name\":\"youzhanHotel\", \"ub\":5, \"lb\":5},{\"name\":\"cheapoairFlight\", \"ub\": 300, \"lb\": 5},{\"name\":\"airtickets24Flight\", \"ub\": 200, \"lb\": 5},{\"name\":\"mangoFlight\", \"ub\": 200, \"lb\": 5}, {\"name\":\"ctripRail\", \"ub\": 50, \"lb\": 5},{\"name\":\"raileuropeRail\", \"ub\": 50, \"lb\":5}, {\"name\":\"pricelineFlight\", \"ub\": 300, \"lb\":5}, {\"name\":\"agodaHotel\", \"ub\": 100, \"lb\":5}, {\"name\":\"kopuFlight\", \"ub\": 50, \"lb\":5}, {\"name\":\"airkxFlight\", \"ub\": 50, \"lb\":5}, {\"name\":\"voyagesRail\", \"ub\": 50, \"lb\":5}]}";
    Json::Value root;
    Json::Reader reader;
    reader.parse(js_str, root);
    if( mode == Regular)
    {
        m_valve->init(root, 2.0);
    }
    else if( mode == Longterm )
    {
        m_valve->init(root, 4.0);
    }

    // 判断task类型
    if(type == Flight)
    {
        m_generator = FlightGenerator::getInstance(m_now, m_today);
    }
    else if(type == Hotel)
    {
        m_generator = HotelGenerator::getInstance(m_now, m_today);
    }
    else if(type == Train)
    {
        m_generator = TrainGenerator::getInstance(m_now, m_today);
    }
    else if(type == None)
    {
        m_generator = new BaseGenerator();
    }
    else
    {
        _ERROR("[In TaskGenerator::getTasks, unknown type!]");
    }

    // 获取所有task的得分
    vector<PAIR> pair_vec;
    //#pragma omp parallel for num_threads(8)
    for(TYPE_DATA::const_iterator it = type_data.begin(); it != type_data.end(); ++it)
    {
        tr1::unordered_map<string, string> task_map = it->second;
        string workload_key = task_map["workload_key"];
        string source = task_map["source"];
        if( filterAssignedTask(workload_key) )
            continue;
        // 过滤暂不分发的源
        //if(source == "ebookers" || source == "cheapoair" || source == "mango" || source == "airtickets24")
        //{
        //    continue;
        //}
        //if(source == "priceline")
        //    continue;
        string updatetime = task_map["updatetime"];
        float price_wave = atof( task_map["price_wave"].c_str() );
        float score = m_generator->getTaskScore(workload_key, updatetime, price_wave);
        // 考虑惩罚分数
        score += getPenaltyScore(workload_key);
        pair_vec.push_back( make_pair(workload_key, score) );
        // 插入该task的外部得分
        if(in_tasks.find(workload_key) != in_tasks.end())
            score += in_tasks[workload_key];
    }

    // 排序
    sort( pair_vec.begin(), pair_vec.end(), cmp );
    
    // 获得任务
    //#pragma omp parallel for
    for(int i = 0; i < pair_vec.size(); i++)
    {
        string workload_key = pair_vec[i].first;
        //_INFO("workload_key is %s", workload_key.c_str());
        float score = pair_vec[i].second;
        string source, crawl_day;
        parseWorkloadKey(workload_key, source, crawl_day);
        if(m_valve->exceed(source))
        {
            //_INFO("workload_key is %s, thread id is %d", workload_key.c_str(), omp_get_thread_num());
            int index = m_valve->getIndexBySec(source);
            // 计数加1
            m_valve->plus(source);
            // 存储该task
            out_tasks[workload_key] = score;
            cout << workload_key << "\t" << score << endl;
            m_key_to_content[workload_key] = m_generator->key2Content(workload_key);
        }
        //_INFO("workload_key %s DONE!", workload_key.c_str());
    }

    _INFO("IN TaskGenerator::getTask, sort ok!");
    return true;
}


bool TaskGenerator::getLongtermTasks(const TYPE_DATA& type_data, TASK& out_tasks, Type type, Mode mode)
{
    // 首先执行长期任务共有逻辑
    TASK in_tasks;
    //#pragma omp parallel for num_threads(8)
    //for(int i = 0; i < type_data.size(); ++i)
    for(TYPE_DATA::const_iterator it = type_data.begin(); it != type_data.end(); ++it)
    {
        tr1::unordered_map<string, string> task_map = it->second;
        string workload_key = task_map["workload_key"];
        string updatetime = task_map["updatetime"];
        float score = getLongtermTaskScore(workload_key, updatetime);
        in_tasks[workload_key] = score;
    }
    _INFO("getLongtermTasks ok!");
    // 然后根据各类型分别计算得分
    getTasks(type_data, in_tasks, out_tasks, type, mode);
}


float TaskGenerator::getPenaltyScore(const string& workload_key)
{
    float score = 0.0;
    score += feedbackTaskError(workload_key);
    return score;
}

bool TaskGenerator::initData()
{
    TYPE_DATA type_data;
    m_regular_data[Flight] = type_data;
    m_regular_data[Hotel] = type_data;
    m_regular_data[Train] = type_data;
    m_longterm_data[Flight] = type_data;
    m_longterm_data[Hotel] = type_data;
    m_longterm_data[Train] = type_data;
}

bool TaskGenerator::parseWorkloadKey(const string& workload_key, string& source, string& crawl_day)
{
    if( workload_key.find("Flight") != string::npos )
    {
        vector<string> vec;
        SplitString(workload_key, "_", &vec);
        source = vec[2];
        crawl_day = vec[3];
    }
    if( workload_key.find("Hotel") != string::npos )
    {
        vector<string> vec;
        SplitString(workload_key, "|", &vec);
        source = vec[2];
        crawl_day = vec[4];
    }
    if( workload_key.find("Rail") != string::npos )
    {
        vector<string> vec;
        SplitString(workload_key, "_", &vec);
        source = vec[2];
        crawl_day = vec[3];
    }
}

bool TaskGenerator::connect2DB(MYSQL* mysql, const string& host, const string& user, const string& passwd, const string& db)
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


bool TaskGenerator::readData(Type type, const string& host, const string& user, const string& passwd, const string& db)
{
    string sql;
    if(type == Flight)
    {
        sql = "SELECT workload_key, source, updatetime, price_wave FROM flight_task_monitor";
    }
    if(type == Hotel)
    {
        sql = "SELECT workload_key, source, updatetime, price_wave FROM room_task_monitor";
    }
    if(type == Train)
    {
        sql = "SELECT workload_key, source, updatetime, price_wave FROM train_task_monitor";
    }

    // 只读取最近更新的
    if(m_read_num != 0)
    {
        string since_time = (m_now - TimeSpan(0, 0, 10, 0)).ToString("yyyy-MM-dd HH:mm:ss");
        sql += " where updatetime != 'NULL' and updatetime > '" + since_time + "'";
    }

    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        return false;
    }   
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
                tr1::unordered_map<string, string> task_map;
                string workload_key = row[0];
                string source = row[1];
                string updatetime = row[2];
                string price_wave = row[3];
                task_map["workload_key"] = workload_key;
                task_map["source"] = source;
                task_map["updatetime"] = updatetime;
                task_map["price_wave"] = price_wave;
                // 判断是否为长期任务
                int ret = isLongtermTask(workload_key, type);
                if( 0 == ret )
                {
                    m_longterm_data[type][workload_key] = task_map;
                }
                else if( 1 == ret )
                {
                    m_regular_data[type][workload_key] = task_map;
                }
            }   
        }   
        mysql_free_result(res);
    }
    _INFO("[IN TaskGenerator::readData, read %d type data ok!]", type);
    mysql_close(mysql);
    delete mysql;
    return true;
}


int TaskGenerator::isLongtermTask(const string& workload_key, Type type)
{
    // 获得crawl_day
    string crawl_day = "";
    vector<string> vec;
    if( Flight == type )
    {
        SplitString(workload_key, "_", &vec);
        if( vec.size() < 4 )
        {
            _ERROR("[ In TaskGenerator::isLongtermTask, Wrong workload_key, %s! ]", workload_key.c_str() );
            return -1;
        }
        crawl_day = vec[3];
    }
    else if( Hotel == type )
    {
        SplitString(workload_key, "|", &vec);
        if( vec.size() != 5 )
        {
            _ERROR("[ In TaskGenerator::isLongtermTask, Wrong workload_key, %s! ]", workload_key.c_str() );
            return -1;
        }
        crawl_day = vec[4];
    }
    // 判断是否长期
    DateTime crawl_dt = DateTime::Parse(crawl_day, "yyyyMMdd");
    TimeSpan ts = crawl_dt - m_today;
    int interval_days = ts.GetDays();
    if( interval_days < 70 && interval_days > 30)
    {
        return 0;
    }
    else if( interval_days < 10 && interval_days > 5)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


bool TaskGenerator::isLongtermTimeslot(int timeslot)
{
    if( timeslot < 72 && timeslot > 0)
        return true;
    else
        return false;
}


float TaskGenerator::getLongtermTaskScore(const string& workload_key, const string& updatetime)
{
    // 从未更新，最高优先级
    if(updatetime == "NULL")
        return 200.0;
    // 获取未更新时间（秒）
    DateTime update_dt  = DateTime::Parse(updatetime, "yyyy-MM-dd HH:mm:ss");
    TimeSpan ts = m_now - update_dt;
    int non_update_seconds = ts.GetTotalSeconds();
    // 5天还未更新，赋予极高的得分
    if(non_update_seconds > 5*86400)
        return 100.0;
    else
        return 0.0;
}

float TaskGenerator::feedbackTaskError(const string& workload_key)
{
    if( m_not_crawl_tasks.find(workload_key) == m_not_crawl_tasks.end() )
    {
        return 0.0;
    }
    else
    {
        string updatetime = m_not_crawl_tasks[workload_key];
        DateTime update_dt  = DateTime::Parse(updatetime, "yyyy-MM-dd HH:mm:ss");
        TimeSpan ts = m_now - update_dt;
        int non_update_seconds = ts.GetTotalSeconds();
        // 5天未更新，去尝试
        if( non_update_seconds > 5*86400 )
            return 0.0;
        // 否则随着未更新时间，惩罚逐渐减小
        return -1000.0 * (1 - non_update_seconds/(5.0*86400) );
    }
    /*
    else
    {
        return -1000.0;
    }
    */
}

bool TaskGenerator::filterAssignedTask(const string& workload_key)
{
    if( m_assigned_tasks.find(workload_key) != m_assigned_tasks.end() )
        return true;
    return false;
}


bool TaskGenerator::loadTaskError(const string& host, const string& user, const string& passwd, const string& db)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        _ERROR("[In TaskGenerator::loadTaskError, can't connect to host : %s, db : %s, user : %s, passwd : %s]", host.c_str(), db.c_str(), user.c_str(), passwd.c_str());
        return false;
    }
    ostringstream oss;
    oss << "SELECT workload_key, updatetime FROM task_error_monitor where error_code = 25 or error_code = 27 or error_code = 24";
    int t = mysql_query(mysql, oss.str().c_str());
    if( t != 0 )
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), oss.str().c_str());
        mysql_close(mysql);
        delete mysql;
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if( res )
        {
            while( row = mysql_fetch_row(res) )
            {
                string workload_key = row[0];
                string updatetime = row[1];
                m_not_crawl_tasks[workload_key] = updatetime;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}


bool TaskGenerator::loadAssignedTasks(const string& host, const string& user, const string& passwd, const string& db)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        _ERROR("[In TaskGenerator::loadAssignedTasks, can't connect to host : %s, db : %s, user : %s, passwd : %s]", host.c_str(), db.c_str(), user.c_str(), passwd.c_str());
        return false;
    }

    int hour = m_now.GetHour();
    int minute = m_now.GetMinute();
    int timeslot = (hour*60+minute) / 5;
    ostringstream oss;
    string table_name;
    // 在长期和例行交界处有问题
    if(isLongtermTimeslot(timeslot))
        table_name = "workload_longterm";
    else
        table_name = "workload_" + m_today.ToString("yyyyMMdd");
    oss << "SELECT workload_key From " << table_name << " WHERE timeslot = " << timeslot << " or timeslot = " << timeslot+1;
    
    int t = mysql_query(mysql, oss.str().c_str());
    if( t != 0 )
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), oss.str().c_str());
        mysql_close(mysql);
        delete mysql;
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if( res )
        {
            while( row = mysql_fetch_row(res) )
            {
                string workload_key = row[0];
                m_assigned_tasks[workload_key] = 1;
            }
        }
        mysql_free_result(res);
    }
    mysql_close(mysql);
    delete mysql;
    return true;
}

bool TaskGenerator::readTmpTaskKeys(vector<string>& tasks, const string& sql, const string& host, const string& user, const string& passwd, const string& db)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        _ERROR("[In TaskGenerator::readTmpTaskKeys, can't connect to host : %s, db : %s, user : %s, passwd : %s]", host.c_str(), db.c_str(), user.c_str(), passwd.c_str());
        return false;
    }
    int t = mysql_query(mysql, sql.c_str());
    if( t != 0 )
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        mysql_close(mysql);
        delete mysql;
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if( res )
        {
            while( row = mysql_fetch_row(res) )
            {
                string workload_key = row[0];
                tasks.push_back(workload_key);
            }
        }
        mysql_free_result(res);
    }

    mysql_close(mysql);
    delete mysql;
    return true;
}

bool TaskGenerator::readTmpTasks(vector<string>& tasks, const string& sql, const string& host, const string& user, const string& passwd, const string& db)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        _ERROR("[In TaskGenerator::readTmpTasks, can't connect to host : %s, db : %s, user : %s, passwd : %s]", host.c_str(), db.c_str(), user.c_str(), passwd.c_str());
        return false;
    }
    int t = mysql_query(mysql, sql.c_str());
    if( t != 0 )
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        mysql_close(mysql);
        delete mysql;
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql_use_result(mysql);
        if( 2 != mysql_num_fields(res) )
        {
            return false;
        }
        MYSQL_ROW row;
        if( res )
        {
            while( row = mysql_fetch_row(res) )
            {
                string workload_key = row[0];
                string content = row[1];
                tasks.push_back(workload_key);
                m_key_to_content[workload_key] = content;
            }
        }
        mysql_free_result(res);
    }

    mysql_close(mysql);
    delete mysql;
    return true;
}

bool TaskGenerator::readExpTaskKeys(vector<string>& tasks, const string& host, const string& user, const string& passwd, const string& db)
{
    MYSQL* mysql = (MYSQL*)malloc(sizeof(MYSQL));
    if( !connect2DB(mysql, host, user, passwd, db) )
    {   
        _ERROR("[In TaskGenerator::readExpTasks, can't connect to host : %s, db : %s, user : %s, passwd : %s]", host.c_str(), db.c_str(), user.c_str(), passwd.c_str());
        return false;
    }
    string sql = "SELECT workload_key FROM workload_validate";
    int t = mysql_query(mysql, sql.c_str());
    if( t != 0 )
    {
        _ERROR("[mysql_query error: %s] [error sql: %s]", mysql_error(mysql), sql.c_str());
        mysql_close(mysql);
        delete mysql;
        return false;
    }
    else
    {
        MYSQL_RES* res = mysql_use_result(mysql);
        MYSQL_ROW row;
        if( res )
        {
            while( row = mysql_fetch_row(res) )
            {
                string workload_key = row[0];
                tasks.push_back(workload_key);
            }
        }
        mysql_free_result(res);
    }

    // 删除表
    string del_sql = "TRUNCATE TABLE workload_validate";
    int k = mysql_query(mysql, del_sql.c_str());

    mysql_close(mysql);
    delete mysql;
    return true;
}
