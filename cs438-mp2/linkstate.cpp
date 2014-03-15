#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define MAXBUFLEN 100
#define MANAGERPORT 3490
#define MAX_NODES 17
#define MAXDATASIZE 1000
#define MAX_MESSAGE			20
#define MAX_MESSAGE_SIZE		200
using namespace std;

//node * neighbors;
std::map <int, int> costs;
int nodeID;
string myIP;
//int sockfd;

unsigned portNum;
char buf[MAXBUFLEN];
int cost[MAX_NODES][MAX_NODES];
char neighborip[MAX_NODES][INET6_ADDRSTRLEN];
bool converge = false;
int tal_node = 0;

vector<int> set;
vector<vector<int> > topo_map;
int cell_cost[MAX_NODES];
int cell_from[MAX_NODES];
bool cell_left[MAX_NODES];
int cell_next_hop[MAX_NODES];





typedef struct node_info{
	int node_id;
	char ip_addr[INET6_ADDRSTRLEN];
	int neighbor_cost[MAX_NODES];
	char neighbor_ip[MAX_NODES][INET6_ADDRSTRLEN];
	bool isMessage;
	bool isSetup;
	bool isNewtopo;
	int to;
	char messages[MAX_MESSAGE_SIZE];
	int node_num;
	//int meg[MAX_NODES][MAX_NODES];
} node_info;

typedef struct msg_box{
	bool if_message;
	bool if_new_cost;
	int send_cost[MAX_NODES];
	bool flooded[MAX_NODES];
	char foward_message[MAX_MESSAGE_SIZE];
	int from;
	int to;
	int attach[MAX_NODES];

} msg_box;

node_info  msg_from_manager;

msg_box send_info;
msg_box recv_box;

int recv_sockfd;
	
struct addrinfo recv_hints, *recv_servinfo, *recv_p;
int recv_rv;
//int recv_numbytes;
struct sockaddr_storage recv_their_addr;	socklen_t recv_addr_len;
char recv_s[INET6_ADDRSTRLEN];
char myport[100];




// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int receiveDataFromNode(msg_box *recv_box)
{
    	
	
	//recv_numbytes = recvfrom(recv_sockfd, recv_box, sizeof(*recv_box) , 0,(struct sockaddr *)&recv_their_addr, &recv_addr_len);
	/*if(recv_numbytes > 0){
    		
		printf("listener: got packet from %s\n", inet_ntop(recv_their_addr.ss_family, get_in_addr((struct sockaddr *)&recv_their_addr),recv_s, sizeof recv_s));
		printf("listener: packet is %d bytes long\n", recv_numbytes);
		//buf[numbytes] = '\0';
		//printf("listener: packet contains \"%s\"\n", buf);
	}*/
    
	//close(sockfd);
    
	return recvfrom(recv_sockfd, recv_box, sizeof(*recv_box) , 0,(struct sockaddr *)&recv_their_addr, &recv_addr_len);
}


int sendDataToNode(int destID, string destIP, msg_box message)
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
    	char theirport[100];
	sprintf(theirport, "%d", destID+5000);
	
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
    
	if ((rv = getaddrinfo(destIP.c_str(), theirport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        
		break;
	}
    
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}
    

	if((numbytes = sendto(sockfd, &message, sizeof(message), 0, p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}


    
	freeaddrinfo(servinfo);
    
	//printf("talker: sent %d bytes to %s\n", numbytes, destIP.c_str());
	close(sockfd);
    return 0;
}

int print_topo(){
	//printf("1dfsdfsdf\n");
	for(int i = 1; i < 8; i++){
		for(int j = 1; j < 8; j++){
			printf("%i ", cost[i][j]);
		}
		printf("\n");
	}
	//printf("2dfsdfsdf");
	return 0;
}


int initUDP()
{

	sprintf(myport, "%d", nodeID+5000);
	memset(&recv_hints, 0, sizeof recv_hints);
	recv_hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	recv_hints.ai_socktype = SOCK_DGRAM;
	recv_hints.ai_flags = AI_PASSIVE; // use my IP
    
	if ((recv_rv = getaddrinfo(NULL, myport, &recv_hints, &recv_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recv_rv));
		return 1;
	}
    
	// loop through all the results and bind to the first we can
	for(recv_p = recv_servinfo; recv_p != NULL; recv_p = recv_p->ai_next) {
		if ((recv_sockfd = socket(recv_p->ai_family, recv_p->ai_socktype,
                             recv_p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
        
		if (bind(recv_sockfd, recv_p->ai_addr, recv_p->ai_addrlen) == -1) {
			close(recv_sockfd);
			perror("listener: bind");
			continue;
		}
        
		break;
	}
    
	if (recv_p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
    
	freeaddrinfo(recv_servinfo);
    
	printf("listener: waiting to recvfrom...\n");
	fcntl(recv_sockfd, F_SETFL, fcntl(recv_sockfd, F_GETFL) | O_NONBLOCK);
	return 0;
}

void compute(){

	set.push_back(nodeID);
	cell_left[nodeID] = false;
	topo_map.resize(tal_node+1);
	for(int i = 1; i < tal_node+1; i++){
		if(i != nodeID){
			if(cost[nodeID][i] != -1){
				cell_cost[i] = cost[nodeID][i];
				cell_from[i] = nodeID;
			}else{
				cell_cost[i] = 9999;
				cell_from[i] = -1;
			}
			cell_left[i] = true;
		}
	}
	
	while((int)set.size() != tal_node){
		int smallest = 9999;
		int smallest_id = 0;
		for(int i = 1; i < tal_node+1; i++){
			if(cell_left[i] == true){
				if(cell_cost[i] < smallest){
					smallest = cell_cost[i];
					smallest_id = i;
				}
			}
		}
		set.push_back(smallest_id);
		cell_left[smallest_id] = false;
	
		for(int i = 1; i < tal_node+1; i++){
			if((cost[smallest_id][i] != -1) && (cell_left[i] == true)){
				if(cell_cost[i] > cell_cost[smallest_id] + cost[smallest_id][i]){
					cell_cost[i] = cell_cost[smallest_id] + cost[smallest_id][i];
					cell_from[i] = smallest_id;
				}
			}
		}
		
	}
	/*if(nodeID == 5){
		printf("5 to 4 is from %i ", cell_from[4]);
		printf("%i ", cell_from[cell_from[4]]);
		printf("%i ", cell_from[cell_from[cell_from[4]]]);
		printf("%i \n", cell_from[cell_from[cell_from[cell_from[4]]]]);
	}*/
	cell_cost[nodeID] = 0;
	cell_from[nodeID] = nodeID;
	printf("Topology Map:\n");
	printf("From To Cost Hops\n");
	for(int i = 1; i < tal_node+1; i++){
		if(i != nodeID){
			int from = cell_from[i];
			topo_map[i].push_back(i);
			while(from != nodeID){
				topo_map[i].push_back(from);
				from = cell_from[from];
			}
			
			printf(" %i   %i   %i  :", nodeID, i, cell_cost[i]);
			printf("%i ", nodeID);
			for(int j = topo_map[i].size()-1; j >= 0; j--){
				printf("%i ", topo_map[i][j]);
			}
			cell_next_hop[i] = topo_map[i][topo_map[i].size()-1];
			//printf(" and the next hop is %i", cell_next_hop[i]);
			printf("\n");
		}else{
			printf(" %i   %i   0  :%i\n", nodeID, nodeID, nodeID);
		}
	}
	

}

int main(int argc, char *argv[])
{
    int sockfd;
	//char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    	char managerPort[100];
	sprintf(managerPort, "%d", MANAGERPORT);


	if (argc != 2) {
	    fprintf(stderr,"usage: ./linkstate hostname\n");
	    exit(1);
	}
    	for(int i = 0; i < MAX_NODES; i++){
		for(int j = 0; j < MAX_NODES; j++){
			cost[i][j] = -1;
		}
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    	
	if ((rv = getaddrinfo(argv[1], managerPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
        
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
        
		break;
	}
    
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
    
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
	printf("client: connecting to %s\n", s);
    
	freeaddrinfo(servinfo); // all done with this structure
    
	
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
	while(1){
		//printf("trying to reveive!\n");
	    	if(recv(sockfd, &msg_from_manager , sizeof(msg_from_manager), 0) > 0){
			if(msg_from_manager.isSetup == true){
				//receive my id, my ip address and all the neighbors with their link cost
				printf("my IP address is %s\n", msg_from_manager.ip_addr);
	    			//assign my ip address
	    			myIP = msg_from_manager.ip_addr;
	    			//assign my node ID
	    			nodeID = msg_from_manager.node_id;
				tal_node = msg_from_manager.node_num;
				initUDP();
				printf("my ID is %i\n",nodeID);
	    			for(int i = 1; i < MAX_NODES; i++){
					cost[nodeID][i] = msg_from_manager.neighbor_cost[i];
					if(msg_from_manager.neighbor_cost[i] != -1){
						printf("now linked to node %i with cost %i\n", i, msg_from_manager.neighbor_cost[i]);
						for(int j = 0; j < INET6_ADDRSTRLEN; j++){
							neighborip[i][j] = msg_from_manager.neighbor_ip[i][j];
						}
						
						send_info.from = nodeID;
						send_info.if_message = false;
						send_info.if_new_cost = true;
						send_info.flooded[nodeID] = true;
						for(int j = 0; j < MAX_NODES; j++){
							send_info.send_cost[j] = msg_from_manager.neighbor_cost[j];
						}

						//printf("sending to %s\n", msg_from_manager.neighbor_ip[i]);
						sendDataToNode(i,msg_from_manager.neighbor_ip[i], send_info);
					}else{
						neighborip[i][0] = '\0';
					}
				}
				time_t start = time(NULL);
				time_t time_out = 5;
				while(1){
					if(time(NULL) > start+time_out){
						break;
					}
					
					if(receiveDataFromNode(&recv_box) > 0){
						//printf("there is something comming\n");
						if((recv_box.if_new_cost == true) ){
							//printf("flood from %i:", recv_box.from);
							for(int i = 1; i < MAX_NODES; i++){
								cost[recv_box.from][i] = recv_box.send_cost[i];
								cost[i][recv_box.from] = recv_box.send_cost[i];
								//printf("%i ",recv_box.send_cost[i]);
								if(cost[nodeID][i] != -1){
									//recv_box.flooded[nodeID] = true;
									sendDataToNode(i,msg_from_manager.neighbor_ip[i], recv_box);
								}
							}

						}
			
					}
				}

				print_topo();
				compute();
				converge = true;
				send(sockfd, "Y",1, 0);
			}else if(msg_from_manager.isMessage == true){
				//printf("it's not a message???\n");
				sleep(1);
				msg_box send_box;
				send_box.if_message = true;
				send_box.from = nodeID;
				send_box.to = msg_from_manager.to;
				for(int i = 0; i < MAX_MESSAGE_SIZE-1; i++){
					send_box.foward_message[i] = msg_from_manager.messages[i];
				}
				send_box.foward_message[MAX_MESSAGE_SIZE-1] = '\0';
				for(int i = 0; i < MAX_NODES; i++){
					send_box.attach[i] = 0;
				}
				send_box.attach[0] = nodeID;
				printf("from %i to %i hops %i ", nodeID, msg_from_manager.to, nodeID);
				printf("message %s\n", msg_from_manager.messages);
				sendDataToNode(cell_next_hop[msg_from_manager.to],neighborip[cell_next_hop[msg_from_manager.to]] , send_box);
			}else if(msg_from_manager.isNewtopo == true){
				for(int i = 1; i < MAX_NODES; i++){
					//cost[nodeID][i] = msg_from_manager.neighbor_cost[i];
					if(msg_from_manager.neighbor_cost[i] != cost[nodeID][i]){
						if(msg_from_manager.neighbor_cost[i] == -1){
							printf("no longer linked to node %i\n", i);
						}else{
							printf("now linked to node %i with cost %i\n", i, msg_from_manager.neighbor_cost[i]);
						}
						cost[nodeID][i] = msg_from_manager.neighbor_cost[i];
						
					}
					if(msg_from_manager.neighbor_cost[i] != -1){
						msg_box send_info;
						send_info.from = nodeID;
						send_info.if_message = false;
						send_info.if_new_cost = true;
						for(int j = 0; j < MAX_NODES; j++){
							send_info.send_cost[j] = msg_from_manager.neighbor_cost[j];
						}
						//printf("sending to %s\n", msg_from_manager.neighbor_ip[i]);
						sendDataToNode(i,neighborip[i], send_info);
					}
				}
				time_t start = time(NULL);
				time_t time_out = 5;
				while(1){
					if(time(NULL) > start+time_out){
						break;
					}
					msg_box recv_box;
					if(receiveDataFromNode(&recv_box) > 0){
						//printf("there is something comming\n");
						if(recv_box.if_new_cost == true){
							for(int i = 1; i < MAX_NODES; i++){
								cost[recv_box.from][i] = recv_box.send_cost[i];
								cost[i][recv_box.from] = recv_box.send_cost[i];
								if(cost[nodeID][i] != -1){
									sendDataToNode(i,neighborip[i], recv_box);
								}
							}

							
						}
			
					}
				
				}
				print_topo();
				compute();
				converge = true;
				send(sockfd, "Y",1, 0);
			}
		}
	    	
	    	
		msg_box recv_box;
		if(converge == true){
			if(receiveDataFromNode(&recv_box) > 0){
				if(recv_box.if_message == true){
					sleep(1);
					printf("from %i to %i hops", recv_box.from, recv_box.to);
					int empty = 0;
					for(int i = 0; i < MAX_NODES; i++){
						if(recv_box.attach[i] == 0){
							empty = i;
							break;
						}
					}
					recv_box.attach[empty] = nodeID;
					for(int i = 0; i < empty+1; i++){
						printf("%i ", recv_box.attach[i]);
					}
					
					printf("message: %s\n", recv_box.foward_message);
					if(recv_box.to != nodeID){
						sendDataToNode(cell_next_hop[recv_box.to],neighborip[cell_next_hop[recv_box.to]] , recv_box);
					}
				}else if(recv_box.if_new_cost == true){
					for(int i = 1; i < MAX_NODES; i++){
						cost[recv_box.from][i] = recv_box.send_cost[i];
						cost[i][recv_box.from] = recv_box.send_cost[i];
						
						if(cost[nodeID][i] != -1){
							sendDataToNode(i,neighborip[i], recv_box);
						}
					}
					
					print_topo();
					compute();
					//converge = true;
					send(sockfd, "Y",1, 0);
				}
			}
		}
		

		
	}
	return 0;
    
}


