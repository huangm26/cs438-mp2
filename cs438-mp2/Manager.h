#pragma once
#include <stdio.h>
#include <stdlib.h>
class Manager
{
public:
	Manager();
	~Manager();
	//read the topology file 
	void readTopologFile();
	//read message file
	void readMessageFile();
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
};

