#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include "protocol.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

//是否开启DEBUG模式
BOOL DEBUG = FALSE;
//UDP服务监听的广播端口号
int PORT = 26651;

static void send_msg(int sock, char *msg, int msg_len, struct sockaddr *peer_addr, socklen_t peer_addrlen){
	int ret = 0;

	ret = sendto(sock, msg, msg_len, 0, peer_addr, peer_addrlen);
	if (!DEBUG) return;

	if (ret > 0)
	{
		printf("send msg success [%s]\n", msg);
	} else {
		printf("send msg error [%s]\n", msg);
	}
}

/**
* 校验收到的消息是否符合协议格式
* 协议:
* $[cmd]?[name],[port],[name],[port].....$
* 
*/
int verifyReceivedMsg(char *msg){
	if (strlen(msg) <= 2 || msg[0] != '$' || msg[strlen(msg) - 1] != '$')
		return 0;
	return 1;
}

#endif