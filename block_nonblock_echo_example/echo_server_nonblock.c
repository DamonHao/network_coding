/*
 * echo_server_nonblock.c
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
//define _GNU_SOURCE for accept4()
//#define _GNU_SOURCE
//man page says use _GNU_SOURCE, but in socket.h I find it should be __USE_GNU;
//define __USE_GNU for accept4()
#define __USE_GNU
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/poll.h>

#define MAXBUF	1024

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		puts("usage:<program> <port>");
		return 0;
	}
	puts("nonblock echo server up!");
	//set server address;
	int port = atoi(argv[1]);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	int listen_sockfd = -1;
	//Create streaming socket
	if ((listen_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
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

	struct pollfd pfds[1];
	pfds[0].fd = listen_sockfd;
	pfds[0].events = POLLIN;
	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		poll(pfds, sizeof(pfds), -1);
		if (pfds[0].revents & POLLIN)
		{
			int clientfd = accept4(listen_sockfd, (struct sockaddr*) &client_addr,
					&addr_len, SOCK_NONBLOCK);
			pid_t pid;
			if ((pid = fork()) < 0)
			{
				perror("fork");
				exit(errno);
			}
			else if (pid == 0) // in child progress;
			{
				printf("connection from %s:%d up\n", inet_ntoa(client_addr.sin_addr),
						ntohs(client_addr.sin_port));
				struct pollfd client_pfds[1];
				client_pfds[0].fd = clientfd;
				client_pfds[0].events = POLLIN;
				while (1)
				{
					poll(client_pfds, sizeof(client_pfds), -1);
					if (client_pfds[0].revents & POLLIN)
					{
						char buffer[MAXBUF];
						int recv_num = recv(clientfd, buffer, MAXBUF, 0);
						if (recv_num > 0)
						{
							printf("recevied data size: %d\n", recv_num);
							//FIXME: may send less than require;
							int send_num = send(clientfd, buffer, recv_num, 0);
							if (send_num < 0)
							{
								perror("send data error");
							}
						}
						else if (recv_num == 0)
						{
							close(clientfd);
							printf("connection from %s:%d down\n",
									inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
							break;
						}
						else
						{
							perror("received data error");
							exit(errno);
						}
					}
				}
			}
			else // in parent progress
			{
				close(clientfd);
			}
		}
	}
	return 0;
}
