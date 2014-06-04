/*
 * test_libev_cppapi.cc
 *
 *  Created on: Jun 4, 2014
 *      Author: damonhao
 */

#include <ev++.h>

#include <stdio.h>

class MyClass
{
public:
	MyClass()
	{
		io_watcher_.set<MyClass, &MyClass::io_cb>(this);
		io_watcher_.set(STDIN_FILENO, ev::READ);
	}
	void start()
	{
		io_watcher_.start();
	}
private:
	void io_cb(ev::io &io_watcher, int revents)
	{
		puts("io cb\n");
		io_watcher.stop();
	}
	ev::io io_watcher_;
};

int main()
{
	ev::default_loop loop;
	MyClass my_io;
	my_io.start();
	loop.run();
	return 0;
}

