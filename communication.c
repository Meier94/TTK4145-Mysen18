#include "communication.h"

//TCP Client list
static client_t clientList[MAX_NODES];
static int numClients = 0;
void					  cl_add  (client_t client);
void				   cl_remove  (int index);
void				cl_removeAll  ();



void* thr_tcp_com_cycle(void* arg){
	msg_t msg;

	while(1){
		for(int i = 0; i < numClients;i++){
			client_t client = clientList[i];
			msg.data[0] = MSGID_REQUEST;
			msg.length = 1;
			tcp_send(&client,&msg);
			//Waiting for response from slave
			int ret = tcp_receive(&client, &msg, 2);
			if(ret > 0){
				printf("%s\n", msg.data);
			}
			else {
				//Ingen melding mottatt til timeouten gitt ut
				printf("Disconnected\n");
				cl_remove(i);
			}
			sleep(1);
		}
	}
	return NULL;
}

void* thr_tcp_listen(void* arg){
	msg_t msg;
	client_t client = {.conn = (int)arg};
	printf("listening: %d\n", client.conn);

	while (true){
		//Waiting for message from master
		int ret = tcp_receive(&client, &msg, 3);
		if (ret <= 0){
			printf("No response from master, likely gone\n");
			continue;
		}
		msg.length = sprintf(msg.data, "Test message 1");
		tcp_send(&client, &msg);

	}
}


void* thr_tcp_accept_conn(void* arg){
	int access_point = tcp_create_access_point();
	client_t client;

	while(true){
		//Probe new connections for 100ms
		if(tcp_get_access_attempts(access_point)){

			//There are incoming connections waiting to be accepted
			while(tcp_pop_access_attempt(access_point, &client)){
				cl_add(client);
				printf("client added: %s\n", ip_to_string(client.ip));
			}

		}
		//No attempts made
	}

	close(client.conn);
}



//Undersøk hva som skjer i communication cycle når vi gikk over til statisk array som ikke er double buffret
void cl_add(client_t client){
	clientList[numClients] = client;
	numClients++;
}

void cl_remove(int index){
	if(index >= numClients){
		return;
	}
	close(clientList[index].conn);

	for(int i = index; i < numClients-1; i++){
		clientList[i] = clientList[i+1];
	}
	numClients--;
}

void cl_removeAll() {
	for (int i = 0; i < numClients; i++) {
		close(clientList[i].conn);
	}
	numClients = 0;
}
