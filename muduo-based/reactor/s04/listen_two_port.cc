/*
 *Add multiple listening socket to a IO thread.
 *
 */

#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <stdio.h>

void newConnection1(int sockfd, const muduo::InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "1\n", 2);
  muduo::sockets::close(sockfd);
}

void newConnection2(int sockfd, const muduo::InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "2\n", 2);
  muduo::sockets::close(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());


  muduo::EventLoop loop;

  muduo::InetAddress listenAddr1(9981);
  muduo::Acceptor acceptor1(&loop, listenAddr1);
  acceptor1.setNewConnectionCallback(newConnection1);
  acceptor1.listen();

  muduo::InetAddress listenAddr2(9982);
  muduo::Acceptor acceptor2(&loop, listenAddr2);
  acceptor2.setNewConnectionCallback(newConnection2);
  acceptor2.listen();

  loop.loop();
}
