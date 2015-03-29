#ifndef C___helper_h
#define C___helper_h

class SensorData {
private:
    double temp;
    long time;
public:
    SensorData() {}
    SensorData(double temp, long time) {
        this -> temp = temp;
        this -> time = time;
    }
    double getTemp() {
        return temp;
    }
    long getTime() {
        return time;
    }
    bool isOutOfDate(long currentTime, long duration) {
        if (currentTime - time >= duration)
            return false;
        return true;
    }
};

#endif