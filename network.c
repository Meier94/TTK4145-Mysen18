#include "network.h"

typedef struct connection{
	int sockfd;	
}connection_t;

#define BUFLEN 1024
static ipv4 my_ip;
static connection_t connectionList[MAX_NODES];
static int numConnections = 0;

int tcp_create_acceptance_socket();
int tcp_openConnection(ipv4 ip, int port);
int tcp_get_connection_queue(int sockfd);
void tcp_create_connections_from_queue(int sockfd);

void error(char *s){
	perror(s);
	exit(1);
}


//denne burde ikke finnes btw
void send_msg_request(int sockfd){
	message_t message;
	message.data[0] = MSGID_REQUEST;
	message.length = 1;
	if (write(sockfd, message.data, message.length) < 0) {
		error("Couldnt write to master 1\n");
	}
}


//TESTET
// -> virker robust
void tcp_send(int sockfd, message_t* msg){
	if(write(sockfd, msg->data, msg->length) < 0){
		error("Could not write tcp message");
	}
}



//TESTET
// -> venter til timeout om den ikke får noen melding og ret 0
// -> dersom avsenderen avslutter fra sin side får man som regel ECONNRESET og den ret 0 med en gang(tatt høyde for)
// -> virker robust
int receive_tcp(int sockfd, message_t* msg, uint32_t timeout){
	int n;

	unsigned int attempts = 1;
	while(attempts <= timeout*10){
		n = recv(sockfd, msg->data, BUFLEN, 0);
		if(n < 0){
			if (errno != EWOULDBLOCK && errno != EAGAIN) {
				if(errno == ECONNRESET){
					//Disconnected
					printf("ECONNRESET\n");
					return 0;
				}
				//Shit hit the fan
				error("Error receiving tcp message");
			}
			attempts++;
			continue;
		}
		//Received message
		msg->length = n;
		return n;
    }
    //Timed out
    return n;
 }



//Hva skal man gjøre i de tilfellene der funksjoner som socket() og bind() returnerer dårlige verdier?
//burde håndteres ved å måtte prøve på nytt eller liknende.


//Undersøk hva som skjer i communication cycle når vi gikk over til statisk array som ikke er double buffret
void cl_addConnection(connection_t newConnection){
	connectionList[numConnections] = newConnection;
	numConnections++;
}

void cl_removeConnection(int index){
	if(index >= numConnections){
		return;
	}
	close(connectionList[index].sockfd);

	for(int i = index; i < numConnections-1; i++){
		connectionList[i] = connectionList[i+1];
	}
	numConnections--;
}

void cl_removeAll() {
	for (int i = 0; i < numConnections; i++) {
		close(connectionList[i].sockfd);
	}
	numConnections = 0;
}



int tcp_openConnection(ipv4 ip, int port) {
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) printf("ERROR opening socket");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ip.addr;
	serv_addr.sin_port = htons(port);

	int res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (res < 0) {
		// error("ERROR connecting");
		printf("Tried to open connection to %s:%d but failed\n", ip_to_string(ip), port);
		return -1;
	}

	return sockfd;
}



void* thr_tcp_communication_cycle(void* arg){
	message_t msg;

	while(1){
		for(int i = 0; i < numConnections;i++){
			int sockfd = connectionList[i].sockfd;
			send_msg_request(sockfd);
			//Waiting for response from slave
			int ret = receive_tcp(sockfd, &msg, 2);
			if(ret > 0){
				printf("%s\n", msg.data);
			}
			else {
				//Ingen melding mottatt til timeouten gitt ut
				printf("Disconnected\n");
				cl_removeConnection(i);
			}
		}
	}
	return NULL;
}

void* thr_tcp_listen(void* arg){
	message_t msg;
	int sockfd = (int)arg;
	printf("socket: %d\n", sockfd);

	while (true){
		//Waiting for message from master
		int ret = receive_tcp(sockfd, &msg, 2);
		if (ret <= 0){
			continue;
		}
		msg.length = sprintf(msg.data, "Test message 1");
		tcp_send(sockfd, &msg);

	}
}



void* thr_tcp_accept_connections(void* arg){
	int sockfd = tcp_create_acceptance_socket();

	while(true){
		//Probe new connections for 100ms
		if(!tcp_get_connection_queue(sockfd)){
			//No connection attempts received
			continue;
		}
		printf("Received attempt(s) to connect\n");
		//There are incoming connections waiting to be accepted
		tcp_create_connections_from_queue(sockfd);
	}
	close(sockfd);
}



int tcp_get_connection_queue(int sockfd){
	int rc;

	//Make socket sole entry in fdset
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	struct timeval timeout = {.tv_sec = 0, .tv_usec = 1e5};
	if ((rc = select(sockfd + 1, &fdset, NULL, NULL, &timeout)) < 0) {
		error("select failed");
	}
	if (rc == 0) {
		//Timeout
		return 0;
	}
	//TCP client discovered
	return 1;
}

int tcp_create_acceptance_socket(){
	int sockfd, rc, on = 1;
	struct sockaddr_in serv_addr;

	//Create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("ERROR opening socket");
	}
	//Socket descriptor may be reused
	int optval = 1;
	if((rc = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &optval, sizeof(optval)) < 0)){
		close(sockfd);
		error(" failed");
	}
	//Set socket to non blocking
	if((rc = ioctl(sockfd, FIONBIO, (char *)&on)) < 0){
		close(sockfd);
		error("ioctl failed");
	}
	//Bind socket
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(TCP_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		error("ERROR on binding");
	}

	//Set listen backlog to 5 entries
	listen(sockfd,5);

	return sockfd;
}

void tcp_create_connections_from_queue(int sockfd){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd;

	//Iterate over all entries in the socket queue of sockfd
	do {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			if (errno != EWOULDBLOCK && errno != EAGAIN) {
				error("Could not accept tcp request");
			}
			break;
		}

		struct timeval tv = {.tv_sec = 0, .tv_usec = 1e5};
		setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
		
		connection_t newConnection;
		newConnection.sockfd = newsockfd;
		cl_addConnection(newConnection);
	} while (newsockfd != -1);

}



udp_sock udp_open_socket(int type) {
	struct sockaddr_in si_me, si_other;

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(sockfd != -1);

	int optval = 1;
	if(type == BROADCAST){
		setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
		memset((char *)&si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(UDP_PORT);
		inet_aton("255.255.255.255",&si_other.sin_addr);
	}
	else if(type == LISTEN){
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		memset((char *)&si_me, 0, sizeof(si_me));
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(UDP_PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);
		struct timeval tv = {.tv_sec = 0, .tv_usec = 1e5};
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
		int res = bind(sockfd, (struct sockaddr *) &si_me, sizeof(si_me));
		if (res == -1) printf("thr_udpListen:bind\n");
	}


	udp_sock conn = {.sockfd = sockfd, .si_other = si_other};
	return conn;
}




void udp_close_socket(udp_sock conn){
	close(conn.sockfd);
}




int udp_recv_msg(udp_sock conn, message_t* msg){
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);

	int res = recvfrom(conn.sockfd, msg->data, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
	if (res == -1) {
		if (errno != EWOULDBLOCK || errno != EAGAIN) {
			error("Error receiving udp message");
		}
		return 0;
	}
	msg->length = res;

	//Check if message came from this machine
	if(si_other.sin_addr.s_addr == my_ip.addr){
		return 0;
	}

	return res;
}




void udp_broadcast(udp_sock conn, message_t* msg){
	int res = sendto(conn.sockfd, msg->data, msg->length, 0, (struct sockaddr *) &conn.si_other, sizeof(struct sockaddr));
	if (res == -1)  error("Could not broadcast udp message");
}




ipv4 ip_get(){
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eno1", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	ipv4 ip;
	ip.addr = (((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
	return ip;
}




char* ip_to_string(ipv4 ip){
	struct in_addr in = {.s_addr = ip.addr};
	return inet_ntoa(in);
}




void network_init(){
	my_ip = ip_get();
}
