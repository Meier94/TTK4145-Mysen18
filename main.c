#include "network.h"
#include <stdio.h>

#include "signal.h"

int count = 0;

void printCtrlC() {
	printf("messages sent: %d\n", count);
	exit(1);
}

int main(){
	network_init();
	signal(SIGINT, printCtrlC);

	uint32_t ip = ip_get();
	printf("IP: %s\n", ip_to_string(ip));

	udp_sock conn = udp_open_socket(BROADCAST);

	message_t msg;
	memset(msg.data, 0, 1024);
	*(uint32_t*)msg.data = ip;
	msg.dataLength = 4;

	while(1){
		udp_broadcast(conn, &msg);
		count++;
		usleep(10000);
	}

}

