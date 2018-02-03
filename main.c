#include "network.h"
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
	printf("Test: %u.%u.%u.%u\n", ip.comp[0],ip.comp[1],ip.comp[2],ip.comp[3]);
	printf("IP: %s\n", ip_to_string(ip));

	udp_sock conn = udp_open_socket(BROADCAST);

	message_t msg;
	memset(msg.data, 0, 1024);
	*(ipv4*)msg.data = ip;
	msg.length = 4;


	pthread_t tcp_accept;
	pthread_t tcp_communicate;
	pthread_create(&tcp_accept, NULL, thr_tcp_accept_connections, NULL);
	pthread_create(&tcp_communicate, NULL, thr_tcp_communication_cycle, NULL);



	while(1){
		udp_broadcast(conn, &msg);
		count++;
		usleep(1e5);
	}

}

