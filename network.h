#pragma once
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <assert.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define MSG_IP_BROADCAST 100

#define BROADCAST 0
#define LISTEN 1

#define SENDER_IS_MASTER 200
#define SENDER_IS_ALONE 201

#define UDP_PORT 4493
#define TCP_PORT 5593

typedef struct udp_sock_struct{
	int sockfd;
	struct sockaddr_in si_other;
}udp_sock;

typedef struct message{
	char data[1024];
	int dataLength;
}message_t;

typedef struct tcp_accept_sock_struct{
	int sockfd
	fd_set fdset;
}tcp_accept_sock;


uint32_t 		ip_get();
char* 			ip_to_string(uint32_t ip);
void 			network_init();

udp_sock 		udp_open_socket(int type);
void 			udp_close_socket(udp_sock conn);
void 			udp_broadcast(udp_sock conn, message_t* msg);
int 			udp_recv_msg(udp_sock conn, message_t* msg);



