#include <iostream>
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
int sockfd;
unsigned portNum;
char buf[MAXBUFLEN];
int cost[MAX_NODES][MAX_NODES];


typedef struct node_info{
	int node_id;
	char ip_addr[INET6_ADDRSTRLEN];
	int neighbor_cost[MAX_NODES];
	char neighbor_ip[MAX_NODES][INET6_ADDRSTRLEN];
	bool isMessage;
	int to;
	char messages[MAX_MESSAGE_SIZE];
} node_info;

typedef struct msg_box{
	int from_id;
	int if_message;
	int if_new_cost;
	int send_cost[MAX_NODES];
	char foward_message[MAX_MESSAGE_SIZE];
} msg_box;

node_info  my_info;






// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int receiveDataFromNode(msg_box* recv_box)
{
    	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
    	char myport[100];
	sprintf(myport, "%d", nodeID+5000);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
    
	if ((rv = getaddrinfo(NULL, myport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
        
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
        
		break;
	}
    
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
    
	freeaddrinfo(servinfo);
    
	//printf("listener: waiting to recvfrom...\n");
    
	addr_len = sizeof their_addr;
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
	numbytes = recvfrom(sockfd, recv_box, sizeof(recv_box) , 0,(struct sockaddr *)&their_addr, &addr_len);
	if(numbytes > 0){
    		
		printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
	}
    
	close(sockfd);
    
	return numbytes;
}


int sendDataToNode(int destID, string destIP, msg_box* message)
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	//int numbytes;
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
    

	if(sendto(sockfd, message, sizeof(message), 0, p->ai_addr, p->ai_addrlen) == -1){
		perror("talker: sendto");
		exit(1);
	}


    
	freeaddrinfo(servinfo);
    
	//printf("talker: sent %d bytes to %s\n", numbytes, destIP.c_str());
	close(sockfd);
    return 0;
}

void print_topo(){
	for(int i = 1; i < MAX_NODES; i++){
		for(int j = 1; j < MAX_NODES; j++){
			printf("%i", cost[i][j]);
		}
		printf("\n");
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
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
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
    
    
    	//receive my id, my ip address and all the neighbors with their link cost
    	recv(sockfd,&my_info , sizeof(my_info), 0);
    
	printf("printing my ip address \n");
    	printf("my IP address is %s\n", my_info.ip_addr);
    
    	//assign my ip address
    	myIP = my_info.ip_addr;
    
    	//assign my node ID
    	nodeID = my_info.node_id;

	
	printf("my ID is %i\n",nodeID);
    	for(int i = 1; i < MAX_NODES; i++){
		cost[nodeID][i] = my_info.neighbor_cost[i];
		if(my_info.neighbor_cost[i] != -1){
			printf("now linked to node %i with cost %i\n", i, my_info.neighbor_cost[i]);
			msg_box send_info;
			send_info.from_id = nodeID;
			send_info.if_message = 0;
			for(int j = 0; j < MAX_NODES; j++){
				send_info.send_cost[j] = my_info.neighbor_cost[j];
			}
			//printf("sending to %s\n", my_info.neighbor_ip[i]);
			sendDataToNode(nodeID,my_info.neighbor_ip[i], &send_info);
		}
	}
	
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
	while(1){
		/*msg_box recv_box;
		if(recv(sockfd,&recv_box , sizeof(recv_box), 0) > 0){
			if(recv_box.if_message == 1){

			}else if(recv_box.if_new_cost == 1){
				
			}
		}*/
		time_t start = time(NULL);
		time_t time_out = 5;
		while(1){
			if(time(NULL) > start+time_out){
				break;
			}
			msg_box recv_box;
			if(receiveDataFromNode(&recv_box) > 0){
				printf("there is message comming\n");
				if(recv_box.if_message == 0){
					for(int i = 1; i < MAX_NODES; i++){
						if(recv_box.send_cost[i] != -1){
							cost[recv_box.from_id][i] = recv_box.send_cost[i];
							cost[i][recv_box.from_id] = recv_box.send_cost[i];
						}
					}
					print_topo();
				}else{
					printf("%s", recv_box.foward_message);
				}
			
			}
		}
		
	}
	return 0;
    
}


