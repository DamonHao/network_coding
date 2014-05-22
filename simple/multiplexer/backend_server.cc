/*
 * backend_server.cc
 *
 *  Created on: May 22, 2014
 *      Author: damonhao
 */
#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

const size_t HEADER_LEN = 3;
const size_t MAX_PACKET_LEN = 255;

class BackendServer : boost::noncopyable
{
public:
	BackendServer(EventLoop *loop, InetAddress listenAddr)
	:server_(loop, listenAddr, "BackendServer")
	{
		server_.setConnectionCallback(bind(&BackendServer::onConnection, this, _1));
		server_.setMessageCallback(bind(&BackendServer::onMessage, this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}
private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		if(conn->connected())
		{
			LOG_INFO << "Conn from" << conn->peerAddress().toIpPort() <<"up";
		}
		else
		{
			LOG_INFO << "Conn from" << conn->peerAddress().toIpPort() <<"down";
		}
	}
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf,Timestamp)
	{
		while(buf->readableBytes() >= HEADER_LEN)
		{
			size_t len = static_cast<uint8_t>(*buf->peek());
			assert(len <= MAX_PACKET_LEN);
			int id = static_cast<uint8_t>(buf->peek()[1]);
			id |= static_cast<uint8_t>(buf->peek()[2] << 8);
			LOG_INFO <<"Receive message from id " << id << ", message size:" << len;
			buf->retrieve(len + HEADER_LEN);
			sendClientMessage(conn, id);
		}
	}

	void sendClientMessage(const TcpConnectionPtr& conn, int id)
	{
		Buffer buf;
		string mesg = "I receive your message\n";
		buf.append(mesg);
		uint8_t header[HEADER_LEN] = {
				static_cast<uint8_t>(mesg.length()),
				static_cast<uint8_t>(id & 0xFF),
				static_cast<uint8_t>((id & 0xFF00) >> 8)//max port 65535
		};
		buf.prepend(header, HEADER_LEN);
		conn->send(&buf);
	}
	TcpServer server_;
};

int main()
{
	const uint16_t listenPort = 9999;
	InetAddress listenAddr(listenPort);
	EventLoop loop;
	BackendServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	return 0;
}



