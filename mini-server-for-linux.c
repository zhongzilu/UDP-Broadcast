#include "common.h"
//获取文件大小
#include <sys/stat.h>
// #include <sys/inotify.h>

//服务列表配置文件路径
char * PATH = "./ssdp.service";

// static void sighandle(int sign){
// 	exit(1);
// }

void parseOptions(int argc, char *argv[]){
	for (int i = 1; i < argc; ++i)
	{
		char * opt = argv[i];
		if(strcmp("-h", opt) == 0 || strcmp("--help", opt) == 0) {
			printf("Usage: \n");
			printf("  %s [options]\n\n", argv[0]);
			printf("OPTIONS:\n");
			printf("  -c, --config : set service config file path\n\n");
			printf("  -p, --port   : set server listening port.\n\n");
			printf("  -d, --debug  : run with debug mode, this will print debug log.\n\n");
			printf("  -h, --help   : print help info.\n\n");
			exit(0);
		}

		else if(strcmp("-p", opt) == 0 || strcmp("--port", opt) == 0){
			if (argv[i+1] != NULL){
				PORT = atoi(argv[i+1]);
				if (PORT < 1025 || PORT > 65535)
				{
					printf("port must in range 1025~65535.\n");
					exit(0);
				}
			}
		}

		else if(strcmp("-d", opt) == 0 || strcmp("--debug", opt) == 0){
			DEBUG = TRUE;
		}

		else if(strcmp("-c", opt) == 0 || strcmp("--config", opt) == 0){
			if (argv[i+1] != NULL){
				PATH = argv[i+1];
			}
		}
	}
}

/*
*获取文件信息中的大小
**/
int file_size(char * filename){
	struct stat statbuf;
	stat(filename, &statbuf);
	return statbuf.st_size;
}

void readServiceConfigFile(char *service, int len){
	if (len <= 0) return;

	FILE *fp = fopen(PATH, "r");
	if (fp == NULL) {
		//not exist file
		printf("not exist file.\n");
		return;
	}

	fgets(service, len, fp);
	if (DEBUG) printf("read config file content: %s\n", service);

	// char buff[len];
	// int index = 0;
	// int count = 0;
	// while(fgets(buff, len, fp) != NULL){
	// 	int i = strlen(buff);
	// 	printf("strlen %d\n", i);
	// 	if (DEBUG) printf("%s", buff);

	// 	if (index + i == len) i -= count;

	// 	memcpy(&service[index], buff, i);
	// 	index += i;
	// 	count += 1;
	// }
	
	fclose(fp);
}

int main(int argc, char *argv[])
{
	
	parseOptions(argc, argv);

	if (DEBUG)
		printf("server listen to port: %d & service config file path: %s\n", PORT, PATH);
	
	int sock;
	struct sockaddr_in own_addr, peer_addr;
	char recv_msg[32] = {0};
	socklen_t peer_addrlen = sizeof(struct sockaddr_in);

	char peer_name[15] = {0};
	int ret = 0;

	// signal(SIGINT, sighandle);
	// signal(SIGTERM, sighandle);

	bzero(&own_addr, sizeof(struct sockaddr_in));
	bzero(&peer_addr, sizeof(struct sockaddr_in));
	own_addr.sin_family = AF_INET;
	own_addr.sin_port = htons(PORT);
	own_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		printf("create sock fail.\n");
		exit(0);
	}

	ret = bind(sock, (struct sockaddr *) &own_addr, sizeof(own_addr));
	if (ret == -1)
	{
		printf("bind addr fail.\n");
		exit(0);
	}

	int size = file_size(PATH);
	if (DEBUG)
		printf("config file size: %d\n", size);

	char services[size];
	if (size > 0)
	{
		readServiceConfigFile(services, size);
	}

	while(TRUE) {
		ret = recvfrom(sock, recv_msg, sizeof(recv_msg), 0, (struct sockaddr *) &peer_addr, &peer_addrlen);
		if (ret < 0)
		{
			printf("\nreceived msg error.\n");
			bzero(recv_msg, sizeof(recv_msg));
			continue;
		}

		inet_ntop(AF_INET, &peer_addr.sin_addr.s_addr, peer_name, sizeof(peer_name));
		if (DEBUG)
			printf("\nreceived msg from [%s]: %s\n", peer_name, recv_msg);

		if (strcmp(EXIT, recv_msg) == 0) {
			printf("client [%s] exit.\n", peer_name);
			bzero(recv_msg, sizeof(recv_msg));
			continue;
		}

		if(!verifyReceivedMsg(recv_msg) && DEBUG)
		{
			printf("verify received msg not passed. [%s]\n", recv_msg);
			send_msg(sock, recv_msg, strlen(recv_msg), (struct sockaddr *)&peer_addr, peer_addrlen);
			bzero(recv_msg, sizeof(recv_msg));
			continue;
		}

		//parse received msg
		if (strcmp(DISCOVER, recv_msg) != 0)
		{
			send_msg(sock, recv_msg, strlen(recv_msg), (struct sockaddr *)&peer_addr, peer_addrlen);
			bzero(recv_msg, sizeof(recv_msg));
			continue;
		}

		if (size > 0)
		{
			bzero(recv_msg, sizeof(recv_msg));
			memcpy(&recv_msg[0], "$resp?", strlen("$resp?"));
			memcpy(&recv_msg[strlen("$resp?")], services, strlen(services));
			memcpy(&recv_msg[strlen(recv_msg)], "$", 1);

			send_msg(sock, recv_msg, strlen(recv_msg), (struct sockaddr *)&peer_addr, peer_addrlen);
			bzero(recv_msg, sizeof(recv_msg));
		} else {
			send_msg(sock, RESP, strlen(RESP), (struct sockaddr *)&peer_addr, peer_addrlen);
		}

		bzero(recv_msg, sizeof(recv_msg));
	}

	send_msg(sock, EXIT, strlen(EXIT), (struct sockaddr *)&peer_addr, peer_addrlen);

	close(sock);

	printf("Done & Exit\n");
	return 0;
}