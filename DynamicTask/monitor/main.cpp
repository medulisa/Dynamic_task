#include "TrainMonitor.hpp"
#include "FlightMonitor.hpp"
#include "RoomMonitor.hpp"

using namespace std;

int main(int argc, const char *argv[])
{
    TrainMonitor tm;
    tm.init();
    tm.readTrain();
    tm.writeTrain();
    tm.free();

    FlightMonitor fm;
    fm.readFlightOneway();
    fm.readFlightRound();
    fm.writeFlight();
    fm.free();

    RoomMonitor rm;
    rm.readRoom();
    rm.writeRoom();
    rm.free();

    return 0;
}
