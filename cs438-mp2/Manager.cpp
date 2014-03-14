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
#include <pthread.h>
#include <unistd.h>

#define MAX_NODES			17
#define MAX_MESSAGE			20
#define MAX_MESSAGE_SIZE		200

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
using namespace std;

int cost[MAX_NODES][MAX_NODES];
char messages[MAX_MESSAGE][MAX_MESSAGE_SIZE];
pthread_t  p_thread[MAX_NODES];      /* thread's structure */
char s[MAX_NODES][INET6_ADDRSTRLEN];
int space[MAX_NODES];
int node_fd[MAX_NODES];
int message_count = 0;
int tol_nodes = 0;
int node_count = 0;
int converge = 0;
	
typedef struct node_info{
	int node_id;
	char ip_addr[INET6_ADDRSTRLEN];
	int neighbor_cost[MAX_NODES];
	char neighbor_ip[MAX_NODES][INET6_ADDRSTRLEN];
	bool isMessage;
	int to;
	char messages[MAX_MESSAGE_SIZE];
} node_info;

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* manage_thread(void *identifier){
	node_info send_info;
	int ID;
	for(int i = 1; i < MAX_NODES; i++){
		if(pthread_self() == p_thread[i]){
			send_info.node_id = i;
			ID = i;
			break;
		}
    	}
	
	for(int j = 0; j < INET6_ADDRSTRLEN; j++){
		send_info.ip_addr[j] = s[ID][j];
	}
	//strcpy(send_info.ip_addr,s[ID]);
	while(1){
		if(node_count == tol_nodes){
			break;
		}	
	}
	for(int i = 1; i < MAX_NODES; i++){
		if(cost[ID][i] != -1){
			send_info.neighbor_cost[i] = cost[ID][i];
			for(int j = 0; j < INET6_ADDRSTRLEN; j++){
				send_info.neighbor_ip[i][j] = s[i][j];
			}
			//strcpy(send_info.neighbor_ip[i],s[i]);
			//printf("%s\n", send_info.neighbor_ip[i]);
		}else{
			send_info.neighbor_cost[i] = -1;
			send_info.neighbor_ip[i][0] = '\0';
		}
	}
	
	send(node_fd[ID], &send_info, sizeof(send_info), 0);
	


	char a;
	recv(node_fd[ID], &a, 1, 0);
	converge++;
	while(1){
		if(converge == tol_nodes){
			break;
		}
	}

	while(1){
		if((int)(messages[message_count][0]-'0') == ID){
			node_info new_message;
			new_message.isMessage = true;
			new_message.to = messages[message_count][3];
			for(int i = 0; i < MAX_MESSAGE_SIZE-3; i++){
				new_message.messages[i] = messages[message_count][i+3];
			}
			new_message.messages[MAX_MESSAGE_SIZE-1] = '\0';
			send(node_fd[ID], &new_message, sizeof(new_message), 0);
			message_count++;
		}
	};
	return NULL;
}

int readTopologFile()
{
	FILE * topology_file;
	int a, b, c, d, e, f;

	topology_file = fopen ("topology.txt","r");
	if(topology_file == NULL){
		perror("Error opening topology file");
		return -1;
	}

	for(int i = 0; i < MAX_NODES; i++){
		for(int j = 0; j < MAX_NODES; j++){
			cost[i][j] = -1;
		}
	}

	while(1)
	{
		d = fscanf(topology_file,"%d", &a);
		e = fscanf(topology_file,"%d", &b);
		f = fscanf(topology_file,"%d", &c);
		if((d == EOF) || (e == EOF) ||(f == EOF)){
			break;
		}
		if(a > tol_nodes){
			tol_nodes = a;
		}
		if(b > tol_nodes){
			tol_nodes = b;
		}
		cost[a][b] = c;
		cost[b][a] = c;

	}
	//printf("%i", tol_nodes);
	fclose(topology_file);
	//printf("%i", cost[3][0]);
	return 0;
	
}

int readMessageFile()
{
	FILE * message_file;

	message_file = fopen ("message.txt","r");
	if(message_file == NULL){
		perror("Error opening message file");
		return -1;
	}

	for(int i = 0; i < MAX_MESSAGE; i++){
		messages[i][0] = '\0';
	}

	for(int i = 0; i < MAX_MESSAGE; i++){
		if(fgets(messages[i] , 200, message_file) == NULL){
			break;
		}
	}

	fclose(message_file);
	return 0;
}

int main(void){
	readTopologFile();
	readMessageFile();

	
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	
	int rv;
	int ID = 1;
	
	for(int i = 0; i < MAX_NODES; i++){
		space[i] = 0;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for node connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		for(int i = 1; i < MAX_NODES; i++){
			if(space[i] == 0){
				ID = i;
				break;
			}
		}
		if(ID <= tol_nodes){
			node_count++;
			space[ID] = 1;
			node_fd[ID] = new_fd;
		
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s[ID], sizeof s[ID]);
			printf("server: got connection from %s\n", s[ID]);

			int err = 0;
			 err = pthread_create(&p_thread[ID], NULL, &manage_thread, NULL);
		}else{
			close(new_fd);
		}
		

	}

	return 0;

}

