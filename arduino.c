#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

int fd = 0;
char cur_msg[100];
char last_msg[100];
unsigned char stop_sign = 0;
pthread_mutex_t lock;

void* display(void* p) {
  char input[100]; 
  while (1) {
    fgets(input, 100, stdin);
    if (strcmp(input, "a\n") == 0)
      write(fd, "a", 1);
    else if (strcmp(input, "b\n") == 0)
      write(fd, "b", 1);
    else if (strcmp(input, "q\n") == 0) {
      pthread_mutex_lock(&lock);
      stop_sign = 1;
      pthread_mutex_unlock(&lock);
      break;
    }
    else {
      pthread_mutex_lock(&lock);
      printf("%s", last_msg);
      pthread_mutex_unlock(&lock);
    }
  }
  pthread_exit(NULL);
}

int main() {
  // first, open the connection
  fd = open("/dev/tty.usbmodem1411", O_RDWR);
  // if open returns -1, something went wrong!
  if (fd == -1) 
    return 0;
  pthread_t t;
  pthread_mutex_init(&lock, NULL);
  // then configure it
  struct termios options;
  tcgetattr(fd, &options);
  cfsetispeed(&options, 9600);
  cfsetospeed(&options, 9600);
  tcsetattr(fd, TCSANOW, &options);
  // read data
  pthread_create(&t, NULL, &display, NULL);
  int index = 0;
  char* c;
  while (1) {
    int byte_read = read(fd, c, 1);
    if (byte_read == 0)
      continue;
    cur_msg[index++] = *c;
    if (*c == '\n') {
      cur_msg[index] = '\0';
      pthread_mutex_lock(&lock);
      strcpy(last_msg, cur_msg);
      pthread_mutex_unlock(&lock);
      index = 0;
    }
    pthread_mutex_lock(&lock);
    if (stop_sign) {
      pthread_mutex_unlock(&lock);
      break;
    }
    pthread_mutex_unlock(&lock);
  }
  pthread_join(t, NULL);
  close(fd);
  return 1;
}