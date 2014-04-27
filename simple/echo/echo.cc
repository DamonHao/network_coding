#include "echo.h"

#include <muduo/base/Logging.h>

#include <boost/bind.hpp>
#include <algorithm>


// using namespace muduo;
// using namespace muduo::net;

void toUpper(muduo::string & msg);
void toROT13(muduo::string & msg);

EchoServer::EchoServer(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& listenAddr)
  : server_(loop, listenAddr, "EchoServer")
{
  server_.setConnectionCallback(
      boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
  server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp time)
{
  muduo::string msg(buf->retrieveAllAsString());
  //toUpper(msg);
  toROT13(msg);
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  conn->send(msg);
}

void toUpper(muduo::string & msg)
{
	std::transform(msg.begin(), msg.end(), msg.begin(),::toupper);
}

void toROT13(muduo::string & msg)
{
	int len = msg.length();
	for(int i = 0; i < len -2; i++)//note the end of string is "\r\n"
	{
		if(msg[i] >= 'a' && msg[i] <= 'z')
		{
			msg[i] = 'a' + (msg[i] - 'a' + 13)%26;
		}
		else if(msg[i] >= 'A' && msg[i] <= 'Z')
		{
			msg[i] = 'A' + (msg[i] - 'A' + 13)%26;
		}
		else
		{
			msg = "Invalid Input!\r\n";
			break;//note to break;
		}
	}
}

