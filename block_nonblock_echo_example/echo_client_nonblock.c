/*
 * echo_client_nonblock.c
 *
 *  Created on: Jun 7, 2014
 *      Author: damonhao
 */

#include <stdio.h> //perror()
#include <string.h> //strlen()
#include <strings.h>//bzero()
#include <errno.h>
#include <unistd.h> //close()
#include <stdlib.h> //exit()

//net
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/poll.h>

#define MAXBUF	1024*1000

int main(int agrc, char *argv[])
{
	if (agrc != 3)
	{
		puts("usage:<program> <server ip> <port>");
		exit(0);
	}
	puts("nonblock echo client up!");
	const char *server_ip = argv[1];
	int server_port = atoi(argv[2]);
	int sockfd;
	//Create a nonblocking streaming socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
	{
		perror("socket");
		exit(errno);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
	//For nonblocking sockfd, connect will return -1 immediately with errno set to EINPROGRESS.
	int ret = connect(sockfd, (struct sockaddr *) &server_addr,
			sizeof(server_addr));
	int saved_errno = (ret == 0) ? 0 : errno;
	if (saved_errno == EINPROGRESS || saved_errno == 0)
	{
		struct pollfd pfds[1];
		pfds[0].fd = sockfd;
		pfds[0].events = POLLOUT | POLLIN;
		while (1)
		{
			poll(pfds, sizeof(pfds), -1);

			if (pfds[0].revents & POLLOUT)
			{
				char input_buff[MAXBUF];
				char message[MAXBUF];
				memset(message, 'a', MAXBUF - 1);
				message[MAXBUF - 1] = '\0';
				int send_num = send(sockfd, message, strlen(message) + 1, 0);
				printf("send num:%d\n", send_num);
				int recv_num = recv(sockfd, input_buff, sizeof(input_buff), 0);
				printf("recv num:%d\n", recv_num);
				close(sockfd);
				break;
			}
		}
	}
	else
	{
		perror("connect");
	}
	return 0;
}

/*
 ./echo_client_nonblock 127.0.0.1 9999
 nonblock echo client up!
 send num:180224
 recv num:1024
 **/
