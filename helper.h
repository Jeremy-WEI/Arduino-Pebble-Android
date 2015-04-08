#ifndef C___Helper_h
#define C___Helper_h

#include <ctime>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <iomanip>

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