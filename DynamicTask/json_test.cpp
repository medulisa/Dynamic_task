#include <iostream>
#include <string>
#include "json/json.h"
using namespace std;

int main()
{
    Json::Value value;
    Json::Reader reader;
    string str = "{\"items\":[{\"name\":\"airberlinFlight\", \"ub\": 300, \"lb\": 5},{\"name\":\"biyiHotel\", \"ub\": 0, \"lb\": 5}]}";
    reader.parse(str, value);
    Json::Value v1 = value["items"];
    cout << v1.size() << endl;
    cout << v1[0]["name"].asString() << endl;
    return 0;
}
