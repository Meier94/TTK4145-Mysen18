#include "network.h"

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


#define BUFLEN 1024
uint32_t my_ip;




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
