#ifndef C___Helper_h
#define C___Helper_h

#include <ctime>

using namespace std;

class SensorData {
public:
    double temp;
    time_t timeStamp;
    SensorData() {}
    SensorData(double temp) {
        this -> temp = temp;
        timeStamp = time(0);
    }
    double getTemp() {
        return temp;
    }
    time_t getTime() {
        return timeStamp;
    }
    bool isOutOfDate(int duration) {
        if (time(0) - timeStamp >= duration)
            return true;
        return false;
    }
};

#endif