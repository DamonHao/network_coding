#include "pubsub.h"
#include <muduo/base/ProcessInfo.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

//use global val because we want to multiplex PubSubClient;
EventLoop* g_loop = NULL;

string g_topic;
string g_content;

string getContent()
{
	struct sysinfo sysInfo;
	const int K_BYTE = 1024;
	if(sysinfo(&sysInfo) < 0)
		{
			return string("error");
		}
		else
		{
			long long virtualMemUsed = sysInfo.totalram - sysInfo.freeram - sysInfo.bufferram;
			//virtualMemUsed += sysInfo.totalswap - sysInfo.freeswap;
			//virtualMemUsed *= sysInfo.mem_unit;
			std::ostringstream ostr;
			ostr << virtualMemUsed/K_BYTE;
			//change from std::string to muduo::string
			string curContent = "the used memory in KByte:" + string(ostr.str().c_str());
			return curContent;
		}
}

void pubProcessState(PubSubClient* client, const string& topic)
{
//	client->publish(topic, ProcessInfo::pidString());
	client->publish(topic, getContent());
}

void connection(PubSubClient* client)
{
	if (client->connected())
	{
//		client->publish(g_topic, g_content);
//		client->stop();
		g_loop->runEvery(2,boost::bind(pubProcessState,client,g_topic));
	}
	else
	{
		g_loop->quit();
	}
}

int main(int argc, char* argv[])
{
	string hostport = argv[1];
	size_t colon = hostport.find(':');
	if (colon != string::npos)
	{
		string hostip = hostport.substr(0, colon);
		uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));
		g_topic = ProcessInfo::hostname();
		//g_content = ProcessInfo::pidString();
		string name = ProcessInfo::username() + "@" + ProcessInfo::hostname();
		name += ":" + ProcessInfo::pidString();
		EventLoop loop;
		g_loop = &loop;
		PubSubClient client(g_loop, InetAddress(hostip, port), name);
		client.setConnectionCallback(connection);
		client.start();
		loop.loop();
//		if (g_content == "-")
//		{
//			EventLoopThread loopThread;
//			g_loop = loopThread.startLoop();
//			PubSubClient client(g_loop, InetAddress(hostip, port), name);
//			client.start();
//
//			string line;
//			while (getline(std::cin, line))
//			{
//				client.publish(g_topic, line);
//			}
//			client.stop();
//		}
//		else
//		{
//			EventLoop loop;
//			g_loop = &loop;
//			PubSubClient client(g_loop, InetAddress(hostip, port), name);
//			client.setConnectionCallback(connection);
//			client.start();
//			loop.loop();
//		}
	}
	else
	{
		printf("Usage: %s hub_ip:port topic content\n", argv[0]);
	}

}
