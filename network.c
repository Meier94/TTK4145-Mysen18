#include "network.h"

static ipv4 my_ip;

//Used to track udp sockets
static struct sockaddr_in udp_recipient;
static int udp_broadcast_socket;
static int udp_receive_socket;
static bool udp_rcv_open = false;
static bool udp_snd_open = false;


void error(char *s){
	perror(s);
	exit(1);
}


//TESTET
// -> virker robust
void tcp_send(client_t* client, msg_t* msg){
	if(send(client->conn, msg->data, msg->length, MSG_CONFIRM) < 0){
		error("Could not write tcp message");
	}
}



//TESTET
// -> venter til timeout om den ikke får noen melding og ret 0
// -> dersom avsenderen avslutter fra sin side får man som regel ECONNRESET og den ret 0 med en gang(tatt høyde for)
// -> virker robust
int tcp_receive(client_t* client, msg_t* msg, uint32_t timeout){
	int n;

	unsigned int attempts = 1;
	while(attempts <= timeout*10){
		n = recv(client->conn, msg->data, BUFLEN, 0);
		if(n < 0){
			if (errno != EWOULDBLOCK && errno != EAGAIN) {
				if(errno == ECONNRESET){
					//Disconnected
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


int tcp_open_conn(client_t* client) {
	struct sockaddr_in serv_addr;

	int conn = socket(AF_INET, SOCK_STREAM, 0);
	if (conn < 0) printf("ERROR opening socket");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = client->ip;
	serv_addr.sin_port = htons(TCP_PORT);

	int res = connect(conn, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (res < 0) {
		// error("ERROR connecting");
		printf("Tried to open connection to %s but failed\n", ip_to_string(client->ip));
		return 0;
	}

	client->conn = conn;

	return 1;
}





int tcp_get_access_attempts(int access_point){
	int rc;

	//Make socket sole entry in fdset
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(access_point, &fdset);

	struct timeval timeout = {.tv_sec = 0, .tv_usec = 1e5};
	if ((rc = select(access_point + 1, &fdset, NULL, NULL, &timeout)) < 0) {
		error("select failed");
	}
	if (rc == 0) {
		//Timeout
		return 0;
	}
	//TCP client discovered
	return 1;
}




int tcp_create_access_point(){
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




int tcp_pop_access_attempt(int access_point, client_t* client){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	//cli_addr can be NULL but it is possible to retrieve ip of recipient if used
	int newconn = accept(access_point, (struct sockaddr *) &cli_addr, &clilen);
	if (newconn < 0) {
		if (errno != EWOULDBLOCK && errno != EAGAIN) {
			error("Could not accept tcp request");
		}
		return 0;
	}

	struct timeval tv = {.tv_sec = 0, .tv_usec = 1e5};
	setsockopt(newconn, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
	
	client->conn = newconn;
	client->ip = cli_addr.sin_addr.s_addr;
	return 1;
}




void udp_open(int type) {
	struct sockaddr_in si_me;

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(sockfd != -1);

	int optval = 1;
	if(type == BROADCAST){
		if(udp_snd_open){
			return;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
		memset((char *)&udp_recipient, 0, sizeof(si_me));
		udp_recipient.sin_family = AF_INET;
		udp_recipient.sin_port = htons(UDP_PORT);
		inet_aton("255.255.255.255",&udp_recipient.sin_addr);
		udp_broadcast_socket = sockfd;
		udp_snd_open = true;

	}
	else if(type == LISTEN){
		if(udp_rcv_open){
			return;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		memset((char *)&si_me, 0, sizeof(si_me));
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(UDP_PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);
		struct timeval tv = {.tv_sec = 0, .tv_usec = 1e5};
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
		int res = bind(sockfd, (struct sockaddr *) &si_me, sizeof(si_me));
		if (res == -1) printf("thr_udpListen:bind\n");
		udp_receive_socket = sockfd;
		udp_rcv_open = true;

	}

	return;
}




void udp_close(int type){
	if(type == BROADCAST && udp_snd_open){
		close(udp_broadcast_socket);
		udp_snd_open = false;
	}
	else if(type == LISTEN && udp_rcv_open){
		close(udp_receive_socket);
		udp_rcv_open = false;
	}
	return;
}




int udp_receive(msg_t* msg){
	struct sockaddr_in sender;
	socklen_t slen = sizeof(sender);

	int res = recvfrom(udp_receive_socket, msg->data, BUFLEN, 0, (struct sockaddr *) &sender, &slen);
	if (res == -1) {
		if (errno != EWOULDBLOCK || errno != EAGAIN) {
			error("Error receiving udp message");
		}
		return 0;
	}
	msg->length = res;

	//Check if message came from this machine
	if(sender.sin_addr.s_addr == my_ip){
		return 0;
	}

	return 1;
}




void udp_broadcast(msg_t* msg){
	int res = sendto(udp_broadcast_socket, msg->data, msg->length, 0, (struct sockaddr *) &udp_recipient, sizeof(struct sockaddr));
	if (res == -1)  error("Could not broadcast udp message");
	return;
}




ipv4 ip_get(){
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eno1", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	return (((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
}




char* ip_to_string(ipv4 ip){
	struct in_addr in = {.s_addr = ip};
	return inet_ntoa(in);
}




void network_init(){
	my_ip = ip_get();
	return;
}
