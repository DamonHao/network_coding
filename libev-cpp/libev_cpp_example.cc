/*
 * test_libev_cppapi.cc
 *
 *  Created on: Jun 4, 2014
 *      Author: damonhao
 */

#include <ev++.h>
#include <stdio.h>

#define BUFFERSIZE 50

class MyClass
{
public:
	MyClass(ev::loop_ref loop, int fd) :
			io_watcher_(loop)
	{
		io_watcher_.set<MyClass, &MyClass::io_cb>(this);
		io_watcher_.set(fd, ev::READ);
	}
	void start()
	{
		io_watcher_.start();
	}
private:
	void io_cb(ev::io &io_watcher, int revents)
	{
		puts("in io_cb");
		char buf[BUFFERSIZE];
		if(gets(buf) != NULL)
		{
			puts(buf);
		}
		else
		{
			puts("read fd error");
		}
		io_watcher.stop();
	}
	ev::io io_watcher_;
};

int main()
{
	ev::default_loop loop;
	MyClass my_io(loop, STDIN_FILENO);
	my_io.start();
	loop.run();
	return 0;
}

