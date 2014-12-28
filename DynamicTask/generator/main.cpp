#include "TaskGenerator.hpp"
#include "CommonFuc.hpp"

int main()
{

    // test json
    
    Json::Value value1, value2,value;
    value1["flight_no"]="su201";
    value1["min_price"]=201;
    value2["flight_no"]="su202";
    value2["min_price"]=202;
    Json::Value value3 = Json::Value(Json::arrayValue);
    value3.append(value1);
    value3.append(value2);
    value["result"]=value3;
    cout << value["result"][0]["flight_no"].asString();
    Json::FastWriter jfw;
    string str = jfw.write(value);
    cout << str;


    
    /*
    Json::Value value;
    Json::Reader reader;
    string str = "{\"items\":[[\"name\":\"airberlinFlight\", \"ub\": 300, \"lb\": 5],[\"name\":\"biyiHotel\", \"ub\": 0, \"lb\": 5]]}";
    reader.parse(str, value);
    Json::Value v = value["items"];
    if( v.isObject() )
        cout << "object" << endl;
    if( v.isArray() )
        cout << "array" << endl;
    if ( v.isString() )
        cout << "string" << endl;
    for(int i = 0; i < v.size(); ++i)
    {
        Json::Value v1 = v[i];
        if( v1.isObject() )
            cout << "object" << endl;
        if( v1.isArray() )
            cout << "array" << endl;
        if ( v1.isString() )
            cout << "string" << endl;
        cout << v1.size() << endl;
        for(int j = 0; j < v1.size(); j++)
        {
            Json::Value v2 = v1[j];
            if ( !v2.isConvertibleTo( Json::objectValue ) )
                cout << "false" << endl;
            if( v2.isArray() )
                cout << "v2 array" << endl;
            if( v2.isString() )
                cout << "v2 string" << endl;
            cout << v2.asString() << endl;
        }
    }
    */

    /*
    TaskGenerator tg;
    //tg.assignTaskByTimeslot(100);
    
    while(true)
    {
        DateTime now = DateTime::Now();
        int minute = now.GetMinute();
        if( minute%10 == 0 )
        {
            // 10分钟执行
            _INFO("******************start generator tasks!*****************");
            tg.writeTask2DB("10.66.115.222", "root", "miaoji@2014!", "workload");
        }
        else
            sleep(3);
    }
    */
    return 0;
}


