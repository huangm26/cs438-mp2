#include "Server.h"


Server::Server()
{
	this->id = 0;
}

Server::Server(int id)
{
	this->id = id;
}

Server::~Server()
{
}

void Server::error(const char *msg)
{
	perror(msg);
	exit(0);
}