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
#include <muduo/net/EventLoopThread.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <string>
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
		server_.setConnectionCallback(boost::bind(&BackendServer::onConnection, this, _1));
		server_.setMessageCallback(boost::bind(&BackendServer::onMessage, this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

	void addCommand(std::string& cmd)
	{
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
//    	connection_->send(cmd);
    	sendClientMessage(connection_, 0, cmd);
    }
	}

private:

	void onConnection(const TcpConnectionPtr& conn)
	{
		MutexLockGuard lock(mutex_);
		if(conn->connected())
		{
			LOG_INFO << "Conn from" << conn->peerAddress().toIpPort() <<"up";
			connection_ = conn;
		}
		else
		{
			connection_.reset();
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

			MutexLockGuard lock(mutex_);//XXX, find a better way to narrow the critical area.
			sendClientMessage(conn, id, "I receive your message\n");
		}
	}

	void sendClientMessage(const TcpConnectionPtr& conn, int id, std::string mesg)
	{
		Buffer buf;
		buf.append(mesg);
		uint8_t header[HEADER_LEN] = {
				static_cast<uint8_t>(mesg.length()),
				static_cast<uint8_t>(id & 0xFF),
				static_cast<uint8_t>((id & 0xFF00) >> 8)//max port 65535
		};
		buf.prepend(header, HEADER_LEN);
//		MutexLockGuard lock(mutex_);
		conn->send(&buf);
	}

	TcpServer server_;
	MutexLock mutex_;
	TcpConnectionPtr connection_;
};

int main()
{
	LOG_INFO << "pid = " << getpid();
	const uint16_t listenPort = 9999;
	InetAddress listenAddr(listenPort);
	EventLoopThread loopThread;
	EventLoop *ioLoop = loopThread.startLoop();
	BackendServer server(ioLoop, listenAddr);
	//TCP server must start in EventLoop ioLoop
	ioLoop->runInLoop(boost::bind(&BackendServer::start, &server));
	std::string line;
  while (std::getline(std::cin, line))
  {
  	server.addCommand(line);
  }
	return 0;
}



