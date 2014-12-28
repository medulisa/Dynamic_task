#include "Valve.hpp"
#include <fstream>
using namespace std;

Valve* Valve::m_pInstance = NULL;

bool Valve::init(const string& config)
{
    std::ifstream is;
    Json::Value root;
    Json::Reader reader;
    
    const char *configure = config.c_str();
    is.open(configure, ios::in);

    if(is.fail())
    {
        cout << "configFile open failed" << endl;
        return false;
    }

    reader.parse(is, root);

    if (root["items"].isNull())
    {
        cout << "no key 'items' in json_object" << endl;
        return false;
    }

    int total = root["items"].size();

    for(int i = 0; i < total; ++i)
    {
        Json::Value source = root["items"][i];
        Section sec(source["lb"].asInt(), source["ub"].asInt(), source["lb"].asInt());
        m_secVec.insert(m_secVec.begin() + i, sec);
        m_secIndex.insert(make_pair(source["name"].asString(),i));
    }
    return true;
}

bool Valve::init(Json::Value& j, float factor)
{
    
    if (j["items"].isNull())
    {
        cout << "no key 'items' in json_object" << endl;
        return false;
    }

    int total = j["items"].size();
    
    for(int i = 0; i < total; ++i)
    {      
        Json::Value source = j["items"][i];
        Section sec(int(factor*source["lb"].asInt()), int(factor*source["ub"].asInt()), int(factor*source["lb"].asInt()));
        m_secVec.insert(m_secVec.begin() + i, sec);
        m_secIndex.insert(make_pair(source["name"].asString(),i));
    }
    return true;
}

bool Valve::init(std::vector<std::string>& sec_vec, std::vector<int>& ub_vec, std::vector<int>& lb_vec)
{
    if (sec_vec.size() != ub_vec.size() || sec_vec.size() != lb_vec.size() || ub_vec.size() != lb_vec.size())
    {
        return false;
    }

    for (int i = 0; i < sec_vec.size(); ++i)
    {   
        Section sec(lb_vec[i], ub_vec[i], lb_vec[i]);
        m_secVec.insert(m_secVec.begin() + i, sec);
        m_secIndex.insert(make_pair(sec_vec[i],i));
    }
    return true;
}

bool Valve::update(const std::string& sec, const int& ub, const int& lb)
{
    int _index = getIndexBySec(sec);
    if (_index == -1)
    {   
        //std::cout << "add source" << std::endl;
        Section section(lb, ub, lb);
        int size = m_secVec.size();
        m_secVec.insert(m_secVec.begin(),section);
        m_secIndex.insert(make_pair(sec, 0));
        return true;
    }
    else if (_index != -1 && _index < m_secVec.size())
    {
        //std::cout << "update source" << std::endl;
        m_secVec[_index].setUpperBound(ub);
        m_secVec[_index].setLowerBound(lb);
        m_secVec[_index].clear();
        return true;
    }
    else
        return false;
}
