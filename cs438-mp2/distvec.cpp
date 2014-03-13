#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAXBUFLEN 100
#define MANAGERPORT 3490
#define MAX_NODES 16

node * neighbors;
std::map <int, int> costs;
int nodeID;
char* myIP;
int sockfd;
unsigned portNum;
char buf[MAXBUFLEN];
typedef struct node_info{
    int node_id;
    string ip_addr;
    int neighbor_cost[MAX_NODES];
    string neighborIP[MAX_NODES];
} node_info;
node_info * my_info;



int connectToManager(int argc, char *argv[])
{
    int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    
	if ((rv = getaddrinfo(argv[1], MANAGERPORT, &hints, &servinfo)) != 0) {
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
    if ((numbytes = recv(sockfd,my_info , sizeof(node_info), 0)) == -1) {
        perror("recv");
    }
    
	printf("printing my ip address \n");
    cout<<my_info.ip_addr;
    //////////////////////////
	//send the message to server
	send(sockfd, "abc", 20, 0);
	close(sockfd);
	
	return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int receiveDataFromNode(char* buf)
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
    
	if ((rv = getaddrinfo(NULL, (int)nodeID+5000, &hints, &servinfo)) != 0) {
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
    
	printf("listener: waiting to recvfrom...\n");
    
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
    
	printf("listener: got packet from %s\n",
           inet_ntop(their_addr.ss_family,
                     get_in_addr((struct sockaddr *)&their_addr),
                     s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);
    
	close(sockfd);
    
	return 0;
}


int sendDataToNode(int destID, string destIP, string message)
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
    
	
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
    
	if ((rv = getaddrinfo(destIP.c_str(), (int)destID+5000, &hints, &servinfo)) != 0) {
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
    
	if ((numbytes = sendto(sockfd, message.c_str(), strlen(message.c_str()), 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
    
	freeaddrinfo(servinfo);
    
	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);
    return 0;
}


int main(int argc, char *argv[])
{
    int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    
	if ((rv = getaddrinfo(argv[1], MANAGERPORT, &hints, &servinfo)) != 0) {
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
    if ((numbytes = recv(sockfd,my_info , sizeof(node_info), 0)) == -1) {
        perror("recv");
    }
    
	printf("printing my ip address \n");
    cout<<my_info.ip_addr;
    
    //assign my ip address
    myIP = my_info.ip_addr.c_str();
    
    //assign my node ID
    nodeID = my_info.node_id;
    
    
    
    
    //////////////////revceive  in the loop
    
    //////////////////////////
	//send the message to server
	send(sockfd, "abc", 20, 0);
	close(sockfd);
	
	return 0;
    
}
