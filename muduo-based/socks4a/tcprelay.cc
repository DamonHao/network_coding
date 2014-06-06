/*
 * tcprelay.cc
 *
 *  Created on: May 28, 2014
 *      Author: damonhao
 */

#include <muduo/net/TcpServer.h>
#include "tunnel.h"

using namespace muduo;
using namespace muduo::net;

EventLoop* g_eventLoop;
InetAddress* g_serverAddr;
//every connection has a tunnel;
std::map<string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		conn->setTcpNoDelay(true);
		TunnelPtr tunnel(new Tunnel(g_eventLoop, *g_serverAddr, conn));
		tunnel->setup();
		tunnel->connect();
		g_tunnels[conn->name()] = tunnel;
	}
	else
	{
		assert(g_tunnels.find(conn->name()) != g_tunnels.end());
		g_tunnels[conn->name()]->disconnect();
		g_tunnels.erase(conn->name());
	}
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
	if (!conn->getContext().empty())
	{
		const TcpConnectionPtr& clientConn =
				boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
		clientConn->send(buf);
	}
}

int main(int argc, char* argv[])
{
	InetAddress serverAddr("127.0.0.1", 9999);
	g_serverAddr = &serverAddr;
	InetAddress listenAddr(3333);
	EventLoop loop;
	g_eventLoop = &loop;
	TcpServer server(g_eventLoop, listenAddr, "ListenServer");
	server.setConnectionCallback(onServerConnection);
	server.setMessageCallback(onServerMessage);
	server.start();
	loop.loop();
}

