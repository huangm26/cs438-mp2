#pragma once
class node
{
public:
	node();
	~node();
	//send data to other node
	void sendDataToNode();
	//notify the neighbor the forwarding table change   DISTANCE VECTOR
	void forwardingTableChange();
	//notift the neighbor the topology change           LINKED STATE
	void topologyChange();
	//determine if everyone's table have converged
	bool hasConverge();
	//inform manager the convergence
	void informConvergence();

private:
	//a linked list of all the neighbors
	node * neighbors;

};

