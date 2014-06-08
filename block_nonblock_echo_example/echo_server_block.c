/*
 * echo_server_block.c
 *
 *  Created on: Jun 8, 2014
 *      Author: damonhao
 */

#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//net
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUF	1024

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
		pid_t pid;
		if ((pid = fork()) < 0)
		{
			perror("fork");
			exit(errno);
		}
		else if (pid == 0) //in child progress
		{
			char buffer[MAXBUF];
			printf("connection from %s:%d up\n", inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
			//Echo back anything received
			int recv_num = 0;
			while ((recv_num = recv(clientfd, buffer, MAXBUF, 0)) > 0)
			{
				printf("recevied data size: %d\n", recv_num);
				send(clientfd, buffer, recv_num, 0);
			}
			if (recv_num < 0)
			{
				puts("received data error");
			}
			close(clientfd);
			printf("connection from %s:%d down\n", inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
			break;
		}
		else // in parent progress
		{
			close(clientfd);
		}
	}
	return 0;
}
