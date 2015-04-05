#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include "Helper.h"

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

define DURATION 3600 // measure in millisecond

using namespace std;

int fd = 0;
double sum = 0;
bool stopSign = false;
vector<SensorData> temps;


void* getSensorData(void* p) {
    string input;
    while (!stopSign) {
        char* ch;
        if (read(fd, ch, 1) = 0)
            continue;
        if (*ch == '\n') {
            SensorData data(atof(input));
            temps.push_back(data);
            sum += data.getTemp();
            for (vector<SensorData> iter = temps.begin(); iter != temps.end(); iter++) {
                if (iter -> isOutOfDate(DURATION)) {
                    sum -= iter -> getTemp();
                    iter = temps.erase(iter);
                    continue;
                }
                break;
            }
            input.clear();
        }
        else
            input += *ch;
    }
}




int main() {
    
    // first, open the connection
    fd = open("/dev/tty.usbmodem1411", O_RDWR);
    
    // if open returns -1, something went wrong!
    if (fd == -1)
        return 0;
    
    // then configure it
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, 9600);
    cfsetospeed(&options, 9600);
    tcsetattr(fd, TCSANOW, &options);
    
}