#include "network.h"
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

	message_t msg;
	ipv4 ip;
	udp_sock conn = udp_open_socket(LISTEN);
	while(1){

		if(!udp_recv_msg(conn, &msg)){
			ip.addr = *(uint32_t*)msg.data;
			printf("%s\n", ip_to_string(ip));
			break;
		}
	}
	int sockfd = tcp_openConnection(ip,TCP_PORT);
	printf("socket:%d\n",sockfd);
	pthread_t tcp_listen;
	pthread_create(&tcp_listen, NULL, thr_tcp_listen, (void*)sockfd);
	while(1){
		printf("loop\n");
		sleep(5);
	}
}