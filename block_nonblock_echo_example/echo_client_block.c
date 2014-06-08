/*
 * echo_client_block.c
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

#define MAXBUF	1024*1000
//#define MAXBUF	20

int main(int agrc, char *argv[])
{
	if (agrc != 3)
	{
		puts("usage:<program> <server ip> <port>");
		exit(0);
	}
	puts("block echo client up!");
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

	char input_buff[MAXBUF];
	char message[MAXBUF];
	memset(message, 'a', MAXBUF - 1);
	message[MAXBUF - 1] = '\0';
	int send_num = send(sockfd, message, strlen(message) + 1, 0);
	printf("send num:%d\n", send_num);
	int recv_num = recv(sockfd, input_buff, sizeof(input_buff), 0);
	printf("recv num:%d\n", recv_num);
	close(sockfd);
	return 0;
}
/*
 ./echo_client_block 127.0.0.1 9999
 block echo client up!
 send num:1024000
 recv num:65664
 */
