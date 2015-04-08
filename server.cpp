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
#include "helper.h"

using namespace std;

#define DURATION 3600

int fd;
char message[100];
vector<SensorData> temps;
pthread_mutex_t lock1, lock2;
bool running = true;
int PORT_NUMBER;

double getMostRecent() {
    pthread_mutex_lock(&lock2);
    double b =  temps[temps.size() - 1].temp;
    pthread_mutex_unlock(&lock2);
    return b;
}

double getExtreme(int duration, bool isHigh) {
    double extreme = getMostRecent();
    pthread_mutex_lock(&lock2);
    int i;
    for (i = temps.size() - 1; i >= 0; i--) {
        if (temps[i].isOutOfDate(duration)) {
            break;
        } else {
            if (isHigh) { // get highest temperature
                if (temps[i].temp > extreme) {
                    extreme = temps[i].temp;
                } 
            } else { // get lower temperature
                if (temps[i].temp < extreme) {
                    extreme = temps[i].temp;
                } 
            }                   
        }        
    }
    if (i > 0) {
        temps.erase(temps.begin(), temps.begin() + i + 1);
    }
    pthread_mutex_unlock(&lock2);
    return extreme;
}

double getLow(int duration) {    
    return getExtreme(duration, false);
}

double getHigh(int duration) {
    return getExtreme(duration, true);
}

double getAverage(int duration) {
    double sum = 0;
    int count = 0;
    pthread_mutex_lock(&lock2);
    int i;
    for (i = temps.size() - 1; i >= 0; i--) {
        if (temps[i].isOutOfDate(duration)) {
            break;
        } else {
            count++;
            sum += temps[i].temp;                  
        }        
    }
    if (i > 0) {
        temps.erase(temps.begin(), temps.begin() + i + 1);
    }
    pthread_mutex_unlock(&lock2);
    if (count == 0) return 0;
    return sum/count;
}


void* startServer(void* p)
{
    // structs to represent the server and client
    struct sockaddr_in server_addr,client_addr;
    
    int sock; // socket descriptor
    char request[1024];
    
    // 1. socket: creates a socket descriptor that you later use to make other system calls
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }
    int temp;
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    
    // configure the server
    server_addr.sin_port = htons(PORT_NUMBER); // specify port number
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);
    
    // 2. bind: use the socket and associate it with the port number
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }
    
    // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
    if (listen(sock, 1) == -1) {
        perror("Listen");
        exit(1);
    }
    
    // once you get here, the server is set up and about to start listening
    printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
    fflush(stdout);
    
    
    // 4. accept: wait here until we get a connection on that port
    int sin_size = sizeof(struct sockaddr_in);
    while (1) {
        int sfd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        
        // buffer to read data into
        
        // 5. recv: read incoming message into buffer
        int bytes_received = recv(sfd,request,1024,0);
        if (bytes_received == 0) {
            close(sfd);
            continue;
        }
        cout << "bytes_received" << bytes_received <<endl;
        // null-terminate the string
        request[bytes_received] = '\0';
        
        cout << request << endl;
        // this is the message that we'll send back
        // for now, it's just a copy of what we received
        stringstream reply;
        reply << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        char *token = strtok(request, " ");
        token = strtok(NULL, " ");
        string command(token + 1);
        if (command.compare("curTemp") == 0) {
            reply << "{\n\"msg\": \"Current temperature is " << getMostRecent() <<"\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
        }
        
        if (command.compare("avgTemp") == 0) {
            reply << "{\n\"msg\": \"Average temperature is " << getAverage(DURATION) <<"\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
        }
        
        if (command.compare("highTemp") == 0) {
            reply << "{\n\"msg\": \"The highest temperature is " << getHigh(DURATION) <<"\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
        }
        
        if (command.compare("lowTemp") == 0) {
            reply << "{\n\"msg\": \"The lowest temperature is " << getLow(DURATION) <<"\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
        }
        
        if (command.compare("showF") == 0) {
            reply << "{\n\"msg\": \"Temperature now in Fahrenheit\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
            int bytes_written = write(fd, "f", 1);
        }
        
        if (command.compare("showC") == 0) {
            reply << "{\n\"msg\": \"Temperature now in Celsius\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
            int bytes_written = write(fd, "c", 1);
        }
        
        if (command.compare("stop") == 0) {
            reply << "{\n\"msg\": \"Now temperature reporting stopped\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
            int bytes_written = write(fd, "s", 1);
        }
        
        if (command.compare("resume") == 0) {
            reply << "{\n\"msg\": \"Now temperature reporting resumed\"\n}\n";
            cout << "Server sent message: " << reply.str() << endl;
            string s = reply.str();
            send(sfd, s.c_str(), s.length(), 0);
            int bytes_written = write(fd, "r", 1);
        }
        
        // 6. send: send the message over the socket
        // note that the second argument is a char*, and the third is the number of chars
        //send(fd, reply, strlen(reply), 0);
        
        // 7. close: close the socket connection
        close(sfd);
    }
    close(sock);
    printf("Server closed connection\n");
    return 0;
}


void* getTem(void* p) {
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, 9600);
    cfsetospeed(&options, 9600);
    tcsetattr(fd, TCSANOW, &options);
    int flag = 0;
    char buf[100];
    int count = -2;
    buf[0] = '\0';
    while (1) {
        pthread_mutex_lock(&lock1);
        if(!running) break;
        pthread_mutex_unlock(&lock1);
        int bytes_read = read(fd, buf, 100);
        if (bytes_read != 0) {
            buf[bytes_read] = '\0';
            
            if (flag) {
                //cout << message << endl;
                try {
                    double temp = atof(message + 1);
                    count++;
                    if (count == 3600) count = 0;
                    if (count >= 0) {
                        pthread_mutex_lock(&lock2);
                        SensorData tempClass(temp);
                        temps.push_back(tempClass);
                        cout << count << " " << temps[count].temp << endl;
                        pthread_mutex_unlock(&lock2);
                        
                    }
                } catch(exception) {
                    cout << "fail" << message << endl;
                }
                message[0] = '\0';
                flag = 0;
            }
            
            strcat(message, buf);
            if (buf[bytes_read - 1] == '\n') {
                flag = 1;
            }
        }
    }
    pthread_mutex_unlock(&lock1);
    
    return 0;
}

int main(int argc, char const *argv[])
{
    PORT_NUMBER = atoi(argv[1]);
    cout << PORT_NUMBER << endl;
    fd = open("/dev/cu.usbmodem1411", O_RDWR);
    // if open returns -1, something went wrong!
    if (fd == -1) return 1;
    pthread_mutex_init(&lock2, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, &getTem, NULL);
    pthread_create(&t2, NULL, &startServer, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    close(fd);
    return 0;
    
}