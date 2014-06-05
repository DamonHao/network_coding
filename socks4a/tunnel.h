/*
 * tunnel.h
 *
 *  Created on: May 28, 2014
 *      Author: damonhao
 */

#ifndef MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H
#define MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H

#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>
//#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class Tunnel: public boost::enable_shared_from_this<Tunnel>, boost::noncopyable
{
public:
	Tunnel(muduo::net::EventLoop* loop, const muduo::net::InetAddress& serverAddr,
			const muduo::net::TcpConnectionPtr& serverConn) :
			client_(loop, serverAddr, serverConn->name()), serverConn_(serverConn)
	{
		LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort() << " <-> "
				<< serverAddr.toIpPort();
	}
	~Tunnel()
	{
		LOG_INFO << "~Tunnel";
	}
	void setup()
	{
		//note shared_from_this() must be called outside constructor;
		client_.setConnectionCallback(
				boost::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
		client_.setMessageCallback(
				boost::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
		//note it is a weak callback; because server Connection, which serverConn_ point to, may
		//live longer than its tunnel;
		serverConn_->setHighWaterMarkCallback(
				boost::bind(&Tunnel::onHighWaterMarkWeak,
						boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2), 10 * 1024 * 1024);
	}

	void tearDown()
	{
		if (serverConn_)
		{
			serverConn_->setContext(boost::any());
			serverConn_->shutdown();
		}
	}

	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}
private:
	void onClientConnection(const muduo::net::TcpConnectionPtr& conn)
	{
		if (conn->connected())
		{
			conn->setTcpNoDelay(true);
			//set client connection's high water mark;
			conn->setHighWaterMarkCallback(
					boost::bind(&Tunnel::onHighWaterMarkWeak,
							boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
					10 * 1024 * 1024);
			serverConn_->setContext(conn);
			if (serverConn_->inputBuffer()->readableBytes() > 0)
			{
				conn->send(serverConn_->inputBuffer());
			}
		}
		else
		{
			tearDown();
		}
	}
	//note TcpConnectionPtr& must be const to match the callback;
	void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
			muduo::net::Buffer* buf, muduo::Timestamp)
	{
		if (serverConn_)
		{
			serverConn_->send(buf);
		}
		else
		{
			buf->retrieveAll();
			abort();
		}
	}

	void onHighWaterMark(const muduo::net::TcpConnectionPtr& conn,
			size_t bytesToSent)
	{
		LOG_INFO << "onHighWaterMark " << conn->name() << " bytes " << bytesToSent;
		disconnect();
	}

	static void onHighWaterMarkWeak(const boost::weak_ptr<Tunnel>& wkTunnel,
			const muduo::net::TcpConnectionPtr& conn, size_t bytesToSent)
	{
		boost::shared_ptr<Tunnel> tunnel = wkTunnel.lock(); //actomic
		if (tunnel)
		{
			tunnel->onHighWaterMark(conn, bytesToSent);
		}
	}

	muduo::net::TcpClient client_;
	muduo::net::TcpConnectionPtr serverConn_;
};

typedef boost::shared_ptr<Tunnel> TunnelPtr;

#endif /* MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H */
