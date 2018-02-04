#pragma once
#include "network.h"
#include <pthread.h>


#define MSGID_IP 100
#define MSGID_REQUEST 101
#define SENDER_IS_MASTER 200
#define SENDER_IS_ALONE 201

void*			  thr_tcp_listen  (void* arg);
void*		   thr_tcp_com_cycle  (void* arg);
void*		 thr_tcp_accept_conn  (void* arg);