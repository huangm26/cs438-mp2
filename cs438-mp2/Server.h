#pragma once
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <winsock.h> 
class Server
{
public:
	Server();
	Server(int id);
	~Server();
	void error(const char *msg);
private:
	int id;
};

