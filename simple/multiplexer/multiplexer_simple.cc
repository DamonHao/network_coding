/*
 * multiplexer_simple.cc
 *
 *  Created on: May 21, 2014
 *      Author: damonhao
 */

#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>
#include <map>
#include <queue>

using namespace muduo;
using namespace muduo::net;
const int MAX_CLIENT_CONNS = 10; //65535
const size_t HEADER_LEN = 3;
const size_t MAX_PACKET_LEN = 255;

class MultiplexServer : boost::noncopyable
{
public:
	MultiplexServer(EventLoop * loop, const InetAddress& listenAddr, const InetAddress backendAddr)
	:	server_(loop, listenAddr, "ListenServer"), backend_(loop, backendAddr, "ConnectBackend")
	{
		backend_.setConnectionCallback(bind(&MultiplexServer::onBackendConnection, this, _1));
		backend_.setMessageCallback(bind(&MultiplexServer::onBackendMessage, this, _1, _2, _3));
		server_.setConnectionCallback(bind(&MultiplexServer::onClientConnection, this, _1));
		server_.setMessageCallback(bind(&MultiplexServer::onClientMessage, this, _1, _2, _3));
	}

	void start()
	{
		backend_.connect();
		server_.start();
	}

private:
	//backend connection establish before client connection!!
	void onBackendConnection(const TcpConnectionPtr& conn)
	{
		if(conn->connected())
		{
			assert(availIds_.empty());
			backendConn_ = conn;
			for(int i = 1; i <= MAX_CLIENT_CONNS; ++i) //note! start from 1;
			{
				availIds_.push(i);
			}
		}
		else
		{
			//Once backend connection is down, release all the client connections;
			backendConn_.reset();
			for(std::map<int, TcpConnectionPtr>::iterator iter = clientConns_.begin();
					iter != clientConns_.end(); ++iter)
			{
				iter->second->shutdown();
			}
			clientConns_.clear();//clean all client conns;
			while(!availIds_.empty())
			{
				availIds_.pop();
			}
		}
	}

	void onBackendMessage(const TcpConnectionPtr& conn, Buffer* buf,Timestamp)
	{
		sendToClient(buf);
	}

	void sendToClient(Buffer* buf)
	{
		while(buf->readableBytes() > HEADER_LEN)
		{
			int len = static_cast<uint8_t>(*buf->peek());
			if(buf->readableBytes() < len + HEADER_LEN)
			{
				break; //buf didn't have enough data;
			}
			else
			{
				int id = static_cast<uint8_t>(buf->peek()[1]);
				id |= (static_cast<uint8_t>(buf->peek()[2])<<8); //id occupy two byte;
				if(id >= 0)
				{
					std::map<int, TcpConnectionPtr>::iterator iter = clientConns_.find(id);
					if(iter != clientConns_.end())
					{
						iter->second->send(buf->peek()+HEADER_LEN, len);
					}
				}
				else
				{
					string cmd(buf->peek()+HEADER_LEN, len);
					LOG_INFO <<"Backend cmd" << cmd;
				}
				buf->retrieve(len + HEADER_LEN);
			}
		}
	}

	void onClientConnection(const TcpConnectionPtr& conn)
	{
		if(conn->connected())
		{
			int id = -1;
			if(!availIds_.empty())
			{
				id = availIds_.front();
				availIds_.pop();
			}
			if(id >= 1)
			{
				clientConns_[id] = conn;
				conn->setContext(id);
			}
			else
			{
				LOG_INFO << "Haven't available id";
				conn->shutdown();
			}
		}
		else
		{
			//release connection resource;
			int id = boost::any_cast<int>(conn->getContext());
			assert(id > 0 && id <= MAX_CLIENT_CONNS);
			if(backendConn_)
			{
				availIds_.push(id);
				clientConns_.erase(id);
			}
			else
			{
				assert(availIds_.empty());
				assert(clientConns_.empty());
			}
		}
	}

	void onClientMessage(const TcpConnectionPtr& conn, Buffer* buf,Timestamp)
	{
		if(!conn->getContext().empty())
		{
			int id = boost::any_cast<int>(conn->getContext());
			sendBackendBuffer(id, buf);
		}
	}

	void sendBackendBuffer(int id, Buffer *buf)
	{
		while(buf->readableBytes() > MAX_PACKET_LEN)
		{
			Buffer packet;
			packet.append(buf->peek(), MAX_PACKET_LEN);
			buf->retrieve(MAX_PACKET_LEN);
			sendBackendPacket(id, &packet);
		}
		if(buf->readableBytes() > 0)
		{
			sendBackendPacket(id, buf);
		}
	}

	void sendBackendPacket(int id, Buffer *buf)
	{
		size_t len = buf->readableBytes();
		assert(len <= MAX_PACKET_LEN);
		uint8_t header[HEADER_LEN] ={
				static_cast<uint8_t>(len),
				static_cast<uint8_t>(id & 0xFF),
				static_cast<uint8_t>((id & 0xFF00) >> 8)//max port 65535
		};
		buf->prepend(header, HEADER_LEN);
		if(backendConn_)
		{
			backendConn_->send(buf);//send will retrieve buf content;
		}
	}

	TcpServer server_;
	TcpClient backend_;
	TcpConnectionPtr backendConn_;
	std::map<int, TcpConnectionPtr> clientConns_;
	std::queue<int> availIds_;
};

//int main(int argc, char* argv[])
//{
//	const uint16_t clientPort = 3333;
//	const uint16_t serverPort = 9999;
//	InetAddress listenAddr(clientPort);
//	InetAddress backendAddr(serverPort);
//	EventLoop loop;
//	MultiplexServer server(&loop, listenAddr, backendAddr);
//	server.start();
//	loop.loop();
//	return 0;
//}

