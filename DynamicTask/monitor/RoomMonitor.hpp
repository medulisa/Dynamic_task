#ifndef _ROOMMONITOR_HPP_
#define _ROOMMONITOR_HPP_ 

#include "BaseMonitor.hpp"

#define ROOM_NUM_FIELDS 8
#define ROOM_MONITOR_NUM_FIELDS 7

class RoomMonitor : public BaseMonitor
{
    public:
        RoomMonitor();
        ~RoomMonitor();
        void free();
        bool readRoom();
        bool writeRoom();
    private:
        CRAWL_DATA m_room_crawl_data;
        MONITOR_DATA m_room_monitor_data;
};

#endif  /*_ROOMMONITOR_HPP_*/


