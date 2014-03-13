#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define MAX_NODES			16
#define MAX_MESSAGE			20
#define MAX_MESSAGE_SIZE	200

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
class Manager
{
public:
	Manager();
	~Manager();
	//read the topology file 
	int readTopologFile();
	//read message file
	int readMessageFile();
	//listen for node connections
	void listenConnection();
	//add topology info from stdin and update the topology
	void addTopology();
	//asign the id to the connected node, tell all the neighbors and link costs to them
	void assignInfo();
	//manager instruct some of nodes to send some data to some other nodes
	void nodeSendData();
	//manager tell the node the topology change
	void topologyChange();
	//check if the whole topology has converged
	bool topologyConverge();
	void* manage_thread(void *identifier);
	void* get_in_addr(struct sockaddr *sa);
	void sigchld_handler(int s);


	int cost[MAX_NODES][MAX_NODES];
	char messages[MAX_MESSAGE][MAX_MESSAGE_SIZE];
	pthread_t  p_thread[MAX_NODES];      /* thread's structure */
	char s[MAX_NODES][INET6_ADDRSTRLEN];
	int space[MAX_NODES];
	int node_fd[MAX_NODES];
	
	typedef struct node_info{
		int node_id;
		char ip_addr[INET6_ADDRSTRLEN];
		int neighbor_cost[MAX_NODES];
		char neighbor_ip[MAX_NODES][INET6_ADDRSTRLEN];
	} node_info;
};


