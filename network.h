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
#include <pthread.h>

#define MAX_NODES 3

#define MSGID_IP 100
#define MSGID_REQUEST 101

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
	int length;
}message_t;

typedef struct tcp_accept_sock_struct{
	int sockfd;
	fd_set fdset;
}tcp_accept_sock;

typedef union ipv4{
	uint32_t addr;
	unsigned char comp[4];
}ipv4;


ipv4			ip_get();
char* 			ip_to_string(ipv4 ip);
void 			network_init();

udp_sock 		udp_open_socket(int type);
void 			udp_close_socket(udp_sock conn);
void 			udp_broadcast(udp_sock conn, message_t* msg);
int 			udp_recv_msg(udp_sock conn, message_t* msg);
int 			tcp_openConnection(ipv4 ip, int port);

void* 			thr_tcp_accept_connections(void* arg);
void* 			thr_tcp_listen(void* arg);
void* 			thr_tcp_communication_cycle(void* arg);



