/*
 * echo_client.c
 *
 *  Created on: Jun 6, 2014
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

#define MAXBUF	1024

int main(int agrc, char *argv[])
{
	if (agrc != 3)
	{
		puts("usage:<program> <server ip> <port>");
		exit(0);
	}
	puts("echo client up!");
	const char *server_ip = argv[1];
	int server_port = atoi(argv[2]);
	int sockfd;
	//Create streaming socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(errno);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))
			!= 0)
	{
		perror("connect");
		exit(errno);
	}

	char output_buff[MAXBUF];
	char input_buff[MAXBUF];
	while (fgets(output_buff, MAXBUF, stdin) != NULL)
	{
		int send_num = send(sockfd, output_buff, strlen(output_buff) + 1, 0);
		if (send_num <= 0)
		{
			perror("socket send");
			exit(errno);
		}
		int recv_num = 0;
		int recv_count = 0;
		while ((recv_num = recv(sockfd, input_buff, sizeof(input_buff), 0)) > 0)
		{
			printf("%s", input_buff);
			recv_count += recv_num;
			if (recv_count == send_num)
			{
				break;
			}
		}
		if (recv_num < 0)
		{
			perror("socket recv");
			exit(errno);
		}
		else if (recv_num == 0)
		{
			break;
		}
	}
	close(sockfd);
	return 0;
}

