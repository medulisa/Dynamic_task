#ifndef _FLIGHTMONITOR_HPP_
#define _FLIGHTMONITOR_HPP_ 

#include "BaseMonitor.hpp"

#define FLIGHT_ONEWAY_NUM_FIELDS 8
#define FLIGHT_ROUND_NUM_FIELDS 10
#define FLIGHT_MONITOR_NUM_FIELDS 7

class FlightMonitor : public BaseMonitor
{
    public:
        FlightMonitor();
        ~FlightMonitor();
        void free();
        bool readFlightOneway();
        bool readFlightRound();
        bool writeFlight();
    private:
        CRAWL_DATA m_flight_crawl_data;
        MONITOR_DATA m_flight_monitor_data;
};

#endif  /*_FLIGHTMONITOR_HPP_*/


