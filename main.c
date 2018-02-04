#include "network.h"
#include "communication.h"

#include <stdio.h>
#include "signal.h"

int count = 0;

void printCtrlC() {
	printf("\33[2K\r");
	printf("messages sent: %d\n", count);
	exit(1);
}

int main(){
	network_init();
	signal(SIGINT, printCtrlC);

	ipv4 ip = ip_get();
	printf("IP: %s\n", ip_to_string(ip));


	pthread_t tcp_accept;
	pthread_t tcp_communicate;
	pthread_create(&tcp_accept, NULL, thr_tcp_accept_conn, NULL);
	pthread_create(&tcp_communicate, NULL, thr_tcp_com_cycle, NULL);


	udp_open(BROADCAST);
	msg_t msg;
	memset(msg.data, 0, 1024);
	*(ipv4*)msg.data = ip;
	msg.length = 4;

	while(1){
		udp_broadcast(&msg);
		count++;
		usleep(1e5);
	}

}

