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
	puts("echo client up!");
	const char *server_ip = "127.0.0.1";
	int server_port = 9999;
	char input_buff[MAXBUF];
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
	char *message = "echo echo";
	int send_num = send(sockfd, message, strlen(message), 0);
	printf("send num:%d\n", send_num);
	int recv_num = recv(sockfd, input_buff, sizeof(input_buff), 0);
	printf("recv num:%d\n", recv_num);
	close(sockfd);
	puts(input_buff);
	return 0;
}

