#ifndef _COMMON_VALVE_H_
#define _COMMON_VALVE_H_
#include <iostream>
#include <vector>
#include <string>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include "../json/json.h"

class Section{
    public:
        Section()
        {
            m_upperbound = 0;
            m_lowerbound = 0;
            m_count = 0;
        };
        Section(const int& c, const int& u, const int& l){
            m_upperbound = u;
            m_lowerbound = l;
            m_count = c;
        };
        ~Section(){};
    public:
        int m_upperbound;
        int m_lowerbound;
        int m_count;

    public:
        void plus(int step = 1)
        {
            m_count += step;
        }
        
        void minus(int step = 1)
        {
            m_count -= step;
        }
        
        //是否超过上限（或小于下限），没超过则返回true;
        bool exceed()
        {   
            if(m_count > m_upperbound)
                return false;
            //if(m_count < m_lowerbound)
                //return false;
            return true;
        }

        void clear()
        {
            //m_count = 0;
            m_count = m_lowerbound;
        }
        
        bool setUpperBound(const int& u)
        {   
            if (u < m_count)
            {
                std::cout << "Warning: New UpperBound Less Than Current Count, Operation Failed" << std::endl;
                return false;
            }
            m_upperbound = u;
            return true;
        }
        
        bool setLowerBound(const int& l)
        {
            if (l > m_count)
            {
                std::cout << "Warning: New LowerBound Larger Than Current Count, Operation Failed" << std::endl;
                return false;
            }
            m_lowerbound = l;
            return true;
        }
        
        bool setCount(const int& c)
        {
            m_count = c;
            return true;
        }
};


class Valve{
    private:
        Valve(){};
        Valve(const Valve& valve){};
        const Valve& operator = (const Valve& valve);
        ~Valve(){};

        static Valve* m_pInstance;

    public:
        static Valve* getInstance()
        {
            if(NULL == m_pInstance)
                m_pInstance = new Valve();
            return m_pInstance;
        }

        static void release()
        {
            if(NULL != m_pInstance)
            {
                delete m_pInstance;
                m_pInstance = NULL;
            }
        }

    public:
        std::vector<Section> m_secVec;
        std::tr1::unordered_map<std::string, int> m_secIndex;

    public:
        int getIndexBySec(const std::string& sec)
        {
            if(m_secIndex.find(sec) != m_secIndex.end())
                return m_secIndex[sec];
            return -1;
        }

    public:
        bool plus(const std::string& sec, int step = 1)
        {
            int _index = getIndexBySec(sec);
            if(_index == -1 || _index >= m_secVec.size())
            {
                //std::cout << "Error in plus: index error for " << sec << "!" << std::endl;
                return false;
            }

            if (m_secVec[_index].exceed())
            {
                m_secVec[_index].plus(step);
                return true;
            }
            
            return false;
        }

        bool minus(const std::string& sec, int step = 1)
        {
            int _index = getIndexBySec(sec);
            if(_index == -1 || _index >= m_secVec.size())
            {
                //cout << "Error in minus: index error for " << sec << "!" << endl;
                return false;
            }
            if (m_secVec[_index].exceed())
            {
                m_secVec[_index].minus(step);
                return true;
            }
            return false;
        }

    public:
        bool init(const std::string& config);
        bool init(Json::Value& j, float factor = 1.0);
        bool init(std::vector<std::string>& sec_vec, std::vector<int>& ub_vec, std::vector<int>& lb_vec);
        bool update(const std::string& sec, const int& ub, const int& lb);

        // 是否超过上限，若不超过返回True
        bool exceed(const std::string& sec)
        {
            int _index = getIndexBySec(sec);
            if(_index == -1 || _index >= m_secVec.size())
            {
                //std::cout << "Error in exceed: index error for " << sec << "!" << std::endl;
                return false;
            }

            //return plus(sec);
            return m_secVec[_index].exceed();
        }

        bool minusExceed(const std::string& sec)
        {
            int _index = getIndexBySec(sec);
            if (_index == -1 || _index >= m_secVec.size())
            {
                //count << "Error in exceed: index error for " << sec << "!" <<endl;
                return false;
            }

            return minus(sec);
            //return true;
        }

        // 设置某个区间的上限
        bool setUpperbound(const std::string& sec, const int& u)
        {
            int _index = getIndexBySec(sec);
            if (_index == -1 || _index >= m_secVec.size())
            {
                //count << "Error in exceed: index error for " << sec << "!" <<endl;
                return false;
            }
            return m_secVec[_index].setUpperBound(u);
        }

        bool setLowerbound(const std::string& sec, const int& l)
        {
            int _index = getIndexBySec(sec);
            if (_index == -1 || _index >= m_secVec.size())
            {
                //count << "Error in exceed: index error for " << sec << "!" <<endl;
                return false;
            }
            return m_secVec[_index].setLowerBound(l);
        }

        // 重置所有计数
        void clearAll()
        {
            for(int i = 0; i < m_secVec.size(); ++i)
                m_secVec[i].clear();
        }

    private:
};

#endif
