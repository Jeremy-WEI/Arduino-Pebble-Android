#ifndef C___Helper_h
#define C___Helper_h

#include <ctime>

class SensorData {
private:
    double temp;
    time_t time;
public:
    SensorData() {}
    SensorData(double temp) {
        this -> temp = temp;
        time = time(0);
    }
    double getTemp() {
        return temp;
    }
    time_t getTime() {
        return time;
    }
    bool isOutOfDate(int duration) {
        if (time(0) - time >= duration)
            return true;
        return false;
    }
};

#endif