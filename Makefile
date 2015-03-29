all: arduino

arduino: arduino.c
	gcc -lpthread -o arduino arduino.c

