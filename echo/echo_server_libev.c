/*
 * echo_server_libev.c
 *
 *  Created on: Jun 8, 2014
 *      Author: damonhao
 */

#include <ev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//net
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUF	1024

typedef struct ClientSocketWatcher
{
	ev_io watcher_;
	uint16_t port_;
	char ip_[16]; //ipv4
} ClientSocketWatcher;

void clientSocketCB(struct ev_loop *loop, ev_io *watcher, int revents)
{
	ClientSocketWatcher *p_client_watcher = (ClientSocketWatcher*) watcher;
	char inputbuff[MAXBUF];
	int clientfd = p_client_watcher->watcher_.fd;
	int recv_num = read(clientfd, inputbuff, MAXBUF);
	if (recv_num > 0)
	{
		printf("recevied %d byte from connection %s:%d\n", recv_num,
				p_client_watcher->ip_, p_client_watcher->port_);
		write(clientfd, inputbuff, (size_t) recv_num);
	}
	else
	{
		if (recv_num < 0)
		{
			perror("received data error");
		}
		close(clientfd);
		printf("connection from %s:%d down\n", p_client_watcher->ip_,
				p_client_watcher->port_);
		ev_io_stop(loop, &(p_client_watcher->watcher_));
		free(p_client_watcher);
	}
}

void listenSocketCB(struct ev_loop *loop, ev_io *watcher, int revents)
{
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int clientfd = accept(watcher->fd, (struct sockaddr*) &client_addr,
			&addr_len);
	ClientSocketWatcher *p_client_watcher = malloc(sizeof(ClientSocketWatcher));
	p_client_watcher->port_ = ntohs(client_addr.sin_port);
	strcpy(p_client_watcher->ip_, inet_ntoa(client_addr.sin_addr));
	//init and activate io_watcher
	ev_io_init(&(p_client_watcher->watcher_), clientSocketCB, clientfd, EV_READ);
	ev_io_start(loop, &(p_client_watcher->watcher_));
	printf("connection from %s:%d up\n", p_client_watcher->ip_,
			p_client_watcher->port_);
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
	uint16_t port = (uint16_t) atoi(argv[1]);

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

	struct ev_loop *loop = ev_default_loop(0);

	ev_io listen_sockfd_wathcer;
	//init and activate io_watcher
	ev_io_init(&listen_sockfd_wathcer, listenSocketCB, listen_sockfd, EV_READ);
	ev_io_start(loop, &listen_sockfd_wathcer);

	//wait for events to arrive
	ev_run(loop, 0);

	return 0;
}
