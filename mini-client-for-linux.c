#include "common.h"

//是否开启输入模式,若开启,则发送的内容需要手动输入,默认关闭
BOOL INPUT = FALSE;

// int g_exit = 0;
// static void sighandle(int sign){
// 	g_exit = -1;
// }

void parseOptions(int argc, char *argv[]){
	for (int i = 1; i < argc; ++i)
	{
		char * opt = argv[i];
		if(strcmp("-h", opt) == 0 || strcmp("--help", opt) == 0) {
			printf("Usage: \n");
			printf("  %s [flag]\n\n", argv[0]);
			printf("  udp_broadcast_client [target_ip] [target_port]\n\n");
			printf("Flags:\n");
			printf("  -p, --port   : set send port.\n\n");
			printf("  -d, --debug  : run with debug mode, this will print debug log.\n\n");
			printf("  -i, --input  : run with input mode, inputed msg will be send.\n\n");
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

		else if(strcmp("-i", opt) == 0 || strcmp("--input", opt) == 0){
			INPUT = TRUE;
		}

		else {
			printf("%s\n", argv[i]);
		}
	}
}

int main(int argc, char *argv[])
{

	parseOptions(argc, argv);

	int sock;
	struct sockaddr_in peer_addr, src_addr;
	const int opt = 1;
	char msg[] = "$discover$";
	socklen_t peer_addrlen = 0;
	//存放回复消息的ip名称
	char src_name[15] = {0};
	int ret = 0;

	// signal(SIGINT, sighandle);
	// signal(SIGTERM, sighandle);

	bzero(&peer_addr, sizeof(struct sockaddr_in));
	bzero(&src_addr, sizeof(struct sockaddr_in));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(PORT);
	peer_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	peer_addrlen = sizeof(struct sockaddr_in);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		printf("create sock fail.\n");
		exit(0);
	}

	ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if (ret == -1)
	{
		printf("set sock to broadcast format fail\n");
		exit(0);
	}

	char input[64] = {0};

	while(TRUE) {
		if (INPUT) {
			printf("Input: ");
			scanf("%s", input);
			if (strcmp("exit", input) == 0) break;
			send_msg(sock, input, strlen(input), (struct sockaddr *)&peer_addr, peer_addrlen);
			bzero(input, sizeof(input));
		} else {
			send_msg(sock, msg, strlen(msg), (struct sockaddr *)&peer_addr, peer_addrlen);
			bzero(msg, sizeof(msg));
		}

		//waiting for server response
		ret = recvfrom(sock, input, sizeof(input), 0, (struct sockaddr *) &src_addr, &peer_addrlen);
		if (ret < 0)
		{
			printf("received msg error.\n");
			bzero(input, sizeof(input));
			continue;
		}

		inet_ntop(AF_INET, &src_addr.sin_addr.s_addr, src_name, sizeof(src_name));
		if (DEBUG)
			printf("received msg from [%s]: %s\n", src_name, input);

		if (!strcmp("exit", input)) break;
		bzero(input, sizeof(input));

		if (!INPUT) break;
		
	}

	memset(msg, 0, sizeof(msg));
	memcpy(msg, "exit", 5);
	send_msg(sock, msg, strlen(msg), (struct sockaddr *)&peer_addr, peer_addrlen);
	close(sock);
	
	printf("Done & Exit\n");
	return 0;
}