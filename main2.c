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

	udp_sock conn = udp_open_socket(LISTEN);
	while(1){

		if(!udp_recv_msg(conn, &msg)){
			printf("%s\n", ip_to_string(*(uint32_t*)msg.data));
			return 0;
		}
	}
}