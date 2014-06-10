/*
 * echo_server_multithread.c
 *
 *  Created on: Jun 8, 2014
 *      Author: damonhao
 */

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//net
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXBUF	1024

typedef struct PassInfo
{
	int socketfd_;
	uint16_t port_;
	char ip_[16]; //ipv4
} PassInfo;

void *threadFunc(void * pass_info)
{
	PassInfo pass_info_ = *((PassInfo *) pass_info);
	printf("connection from %s:%d up\n", pass_info_.ip_, pass_info_.port_);
	//Echo back anything received
	char inputbuff[MAXBUF];
	int recv_num = 0;
	while ((recv_num = read(pass_info_.socketfd_, inputbuff, MAXBUF)) > 0)
	{
		printf("recevied data size: %d\n", recv_num);
		write(pass_info_.socketfd_, inputbuff, recv_num);
	}
	if (recv_num < 0)
	{
		perror("received data error");
	}
	close(pass_info_.socketfd_);
	printf("connection from %s:%d down\n", pass_info_.ip_, pass_info_.port_);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		puts("usage:<program> <port>");
		return 0;
	}
	puts("block echo server up!");
	//set server address;
	int port = atoi(argv[1]);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	int listen_sockfd = -1;

	//Create streaming socket
	if ((listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(errno);
	}

	//bind server address to listen_sockfd
	if (bind(listen_sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr))
			!= 0)
	{
		perror("bind error");
		exit(errno);
	}
	//make listen_sockfd a "listening socket"
	if (listen(listen_sockfd, 20) != 0)
	{
		perror("listen error");
		exit(errno);
	}
	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		int clientfd = accept(listen_sockfd, (struct sockaddr*) &client_addr,
				&addr_len);
		PassInfo pass_info;
		pass_info.socketfd_ = clientfd;
		pass_info.port_ = ntohs(client_addr.sin_port);
		strcpy(pass_info.ip_, inet_ntoa(client_addr.sin_addr));
		pthread_t tid;
		if (pthread_create(&tid, NULL, threadFunc, (void *) &pass_info) != 0)
		{
			perror("thread create");
			exit(errno);
		}
	}
	return EXIT_SUCCESS;
}
