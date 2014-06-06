/*
 * echoserver.c
 *
 *  Created on: Jun 5, 2014
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

int main()
{
	puts("echo server up!");
	//set server address;
	int port = 9999;
	char buffer[MAXBUF];

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
//		puts("bind error");
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
		int clientfd = -1;
		struct sockaddr_in client_addr;
		int addr_len = sizeof(client_addr);
		//accept a connection (creating a data pipe)
		clientfd = accept(listen_sockfd, (struct sockaddr*) &client_addr,
				&addr_len);
		printf("connection from %s:%d up\n", inet_ntoa(client_addr.sin_addr),
				ntohs(client_addr.sin_port));
		//Echo back anything received
		send(clientfd, buffer, recv(clientfd, buffer, MAXBUF, 0), 0);
		//close client fd;
		close(clientfd);
	}
	return 0;
}

