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
#include <poll.h>

#define MAX_NODES 3

#define BROADCAST 0
#define LISTEN 1

#define BUFLEN 1024
#define UDP_PORT 4493
#define TCP_PORT 5593


typedef struct message{
	char data[BUFLEN];
	int length;
}msg_t;

typedef uint32_t ipv4;

typedef struct client{
	int conn;
	ipv4 ip;
}client_t;

void					   error  (char *s);
ipv4					  ip_get  ();
void				network_init  ();
char*				ip_to_string  (ipv4 ip);

void 					udp_open  (int type);
void 				   udp_close  (int type);
int 				 udp_receive  (msg_t* msg);				//ret = 0: *fail*, ret = 1	: *success*
void 			   udp_broadcast  (msg_t* msg);

//TCP

void 					tcp_send  (client_t* client, msg_t* msg);
int 				 tcp_receive  (client_t* client, msg_t* msg, uint32_t timeout);
int 			   tcp_open_conn  (client_t* client);			//ret = 0: *fail*, ret = 1: *success* - Hva skal man gjøre om denne ikke klarer seg
void tcp_poll(client_t* client);
//TCP Server functionality

int		  tcp_pop_access_attempt  (int access_point, client_t* client); //Burde hete tcp_accept_client?
int		 tcp_get_access_attempts  (int access_point);					//Klarer ikke komme på bedre navn til denne, den over kan derfor ikke byttes?
int		 tcp_create_access_point  ();