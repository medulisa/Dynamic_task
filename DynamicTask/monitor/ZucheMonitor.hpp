#ifndef _TRAINMONITOR_HPP_
#define _TRAINMONITOR_HPP_ 

#include "BaseMonitor.hpp"

#define TRAIN_NUM_FIELDS 8
#define TRAIN_MONITOR_NUM_FIELDS 7

class TrainMonitor : public BaseMonitor
{
    public:
        bool readTrain();
        bool writeTrain();
    private:
        CRAWL_DATA m_train_crawl_data;
        MONITOR_DATA m_train_monitor_data;
};

#endif  /*_TRAINMONITOR_HPP_*/


