#include "network.h"
#include "communication.h"
#include <stdio.h>
#include <signal.h>


int count = 0;

void printCtrlC() {
	printf("messages received: %d\n", count);
	exit(1);
}

int main(){

	network_init();
	signal(SIGINT, printCtrlC);

	msg_t msg;
	ipv4 ip;
	udp_open(LISTEN);
	while(1){

		if(udp_receive(&msg)){
			ip = *(uint32_t*)msg.data;
			printf("%s\n", ip_to_string(ip));
			break;
		}
	}
	client_t client;
	client.ip = ip;
	if(!tcp_open_conn(&client)){
		error("Could not open connection");
	}

	printf("socket:%d\n",client.conn);
	pthread_t tcp_listen;
	pthread_create(&tcp_listen, NULL, thr_tcp_listen, (void*)client.conn);
	while(1){
		printf("loop\n");
		sleep(5);
	}
}