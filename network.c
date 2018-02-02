#include "network.h"

#define BUFLEN 1024
uint32_t my_ip;
static connection_t* connectionList = NULL;
static int numConnections = 0;

//void send_msg_request(int i){
//	sockfd = connectionList[i].sockfd;
//	if(sockfd == 0){
//		printf("Tried to write to nonexistant socket\n");
//	}
//
//	message_t message;
//	message.data[0] = MSGID_REQUEST;
//	message.dataLength = 1;
//	if ((rc = write(socket, message.data, message.dataLength)) < 0) {
//		error("Couldnt write to master 1");
//	}
//	if(rc != dataLength){
//		error("Writing returned wrong length\n");
//	}
//}
//
//void receive_tcp(int i){
//	//trenger vel ikke den øverste sjekken? Hvordan er det mulig å 
//	sockfd = connectionList[i].sockfd;
//	if(sockfd == 0){
//		printf("Tried to write to nonexistant socket\n");
//	}
//
//	message_t message;
//	message.data[0] = MSGID_REQUEST;
//	message.dataLength = 1024;
//	int n;
//
//	int attempts = 1;
//	while(attempts <= 3){
//		n = recv(sockfd, message.data, message.dataLength, NULL);
//		if (n <= 0) {
//			attempts++;
//			continue;
//		}
//    	break;
//    }
// }
//


//Hva skal man gjøre i de tilfellene der funksjoner som socket() og bind() returnerer dårlige verdier?
//burde håndteres ved å måtte prøve på nytt eller liknende.

void cl_addConnection(connection_t newConnection){
	connection_t* newList = malloc(sizeof(connection_t*(numConnections+1)));
	for(int i = 0; i < numConnections; i++){
		newList[i] = connectionList[i];
	}
	newList[numConnections] = newConnection;
	numConnections++;
	if(connectionList != NULL){
		free(connectionList);
	}
	connectionList = newList;
}

void cl_removeConnection(int index){
	close(connectionList[i].sockfd);
	connection_t* newList = malloc(sizeof(connection_t*(numConnections-1)));
	for(int i = 0; i < index; i++){
		newList[i] = connectionList[i];
	}
	for(int i = index; i < numConnections-1; i++){
		newList[i] = connectionList[i+1];
	}
	numConnections--;
	if(connectionList != NULL){
		free(connectionList);
	}
	connectionList = newList;
}

void cl_removeAll() {
	for (int i = 0; i < numConnections; i++) {
		close(connectionList[i].sockfd);
	}
	if (connectionList != NULL) {
		free(connectionList);
	}
	connectionList = NULL;
}



int tcp_openConnection(uint32_t ip, int port) {
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) printf("ERROR opening socket");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ip;
	serv_addr.sin_port = htons(port);

	res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (res < 0) {
		// error("ERROR connecting");
		printf("Tried to open connection to %s:%d but failed\n", ip, port);
		return -1;
	}

	return sockfd;
}

void* thr_tcp_accept_connections(void* arg){
	tcp_accept_sock sock = tcp_create_acceptance_socket();


	clilen = sizeof(cli_addr);
	while(!in.cancel){
		if(!tcp_get_connection_queue(sock)){
			//No connection attempts received
			continue;
		}
		//There are incoming connections waiting to be accepted
		tcp_create_connections_from_queue(sock);
	}
	close(sockfd);
}

int tcp_get_connection_queue(tcp_accept_sock sock){
	struct timeval timeout = {.tv_sec = 0, .tv_usec = 1e5};
	if ((rc = select(sock.sockfd + 1, sock.fdset, NULL, NULL, &timeout)) == -1) {
		error("select failed");
	}
	if (rc == 0) {
		return -1;
	}
	return 0;
}

tcp_accept_sock tcp_create_acceptance_socket(){
	int sockfd, newsockfd, port, rc, on = 1, off = 0;

	//Create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("ERROR opening socket");
	}
	//Socket descriptor may be reused
	int optval = 1;
	if((rc = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &optval, sizeof(optval)) < 0){
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
	//Make socket sole entry in fdset
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	tcp_accetp_sock sock = {.sockfd = sockfd, .fdset = fdset};
	return sock;
}

void tcp_create_connections_from_queue(tcp_accept_sock sock){
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	//Iterate over all entries in the socket queue of sockfd
	do {
		newsockfd = accept(sock.sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			if (errno != WOULDBLOCK) {
				error("ERROR on accept");
			}
			//Should not happen with the select check above, but precautionary
			break;
		}

		//Set socket to non blocking
		//-------------------skal denne endres til å sende inn &on istedenfor?
		if ((rc = ioctl(newsockfd, FIONBIO, (char *)&off)) < 0) {
			close(newsockfd);
			error("ioctl failed");
		}

		struct timeval tv = {.tv_sec = 2, .tv_usec = 0};

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
	}

	int res = bind(sockfd, (struct sockaddr *) &si_me, sizeof(si_me));
	if (res == -1) printf("thr_udpListen:bind");


	udp_sock conn = {.sockfd = sockfd, .si_other = si_other};
	return conn;
}




void udp_close_socket(udp_sock conn){
	close(conn.sockfd);
}




int udp_recv_msg(udp_sock conn, message_t* msg){
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);

	//Will only run with a valid IP
	int res = recvfrom(conn.sockfd, msg->data, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
	if (res == -1) {
		if (errno != EWOULDBLOCK || errno != EAGAIN) {
			printf("Error receiving udp message %s,\n", strerror(errno));
		}
		return -1;
	}
	msg->dataLength = res;

	//Chekc if message came from this machine
	uint32_t ip = si_other.sin_addr.s_addr;
	if(ip == my_ip){
		return -1;
	}

	//Vurder å fjerne denne sjekken. Ved testing så ble den aldri utløst med 30% packet loss 10 000*1kB meldinger. Meldinger ble bare ikke plukket opp av recvfrom. 
//	if(*(uint16_t*)msg->data != res){
//		char *ip = inet_ntoa(si_other.sin_addr);
//		printf("Length: %d, %d, %s\n",res,*(uint16_t*)msg->data, ip);
//		return -1;
//	}

	return 0;
}




void udp_broadcast(udp_sock conn, message_t* msg){
	int res = sendto(conn.sockfd, msg->data, msg->dataLength, 0, (struct sockaddr *) &conn.si_other, sizeof(struct sockaddr));
	if (res == -1)  printf("udp_broadcast: sendto()\n");
}




uint32_t ip_get(){
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eno1", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	return (((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
}




char* ip_to_string(uint32_t ip){
	struct in_addr in = {.s_addr = ip};
	return inet_ntoa(in);
}




void network_init(){
	my_ip = ip_get();
}
