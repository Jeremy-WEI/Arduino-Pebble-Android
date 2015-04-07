all: server

server: server.cpp
	c++ -lpthread -o server server.cpp

