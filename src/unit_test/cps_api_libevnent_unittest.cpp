/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */


#include "cps_string_utils.h"

#include "std_time_tools.h"

#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#include <pthread.h>
#include <sys/select.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <sys/socket.h>
#include <event2/event.h>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <list>

const static int PORT=23232;
std::mutex mutex;

struct event_params {
	struct event_base * _event_base;
	std::unordered_map<std::string,event*> _events;
	std::unordered_map<std::string,std::vector<char>> data;
	std::unordered_map<std::string,int> state;
};

void on_rx(int fd, short int events, void *data) {
	//event_params *params = (event_params*)data;
	//std::string key = cps_string::sprintf("%d",(int)fd);
	//auto &buff = params->data[key+"-rx"];

	//int offset = params->state[key+"-rx"];


}

void on_write(int fd, short int events, void * data) {
}

void on_accept(int fd, short int events, void *data) {
	event_params *params = (event_params*)data;
	std::string key = cps_string::sprintf("%d",(int)fd);

	params->_events[key+"-rx"] = event_new(params->_event_base,fd,EV_READ|EV_PERSIST,on_rx,data);

	params->_events[key+"-tx"] = event_new(params->_event_base,fd,EV_WRITE|EV_PERSIST,on_rx,data);

	event_add(params->_events[key+"-rx"],NULL);
	event_add(params->_events[key+"-tx"],NULL);

}

TEST(cps_api_events,init) {
	int s = socket(AF_INET6,SOCK_STREAM,0);

	 struct sockaddr_in6 _addr ;
	 memset(&_addr,0,sizeof(_addr));
	 _addr.sin6_family = AF_INET6;
	 _addr.sin6_port = htonl(PORT);
	 inet_pton(AF_INET6,"::",&(_addr.sin6_addr));

	 int rc = bind(s,(struct sockaddr*)&_addr,sizeof(_addr));
	 ASSERT_TRUE(rc>=0);

	 mutex.lock();

	 size_t cnt = 10000;

	 std::thread th([&]() {
		 mutex.lock();
		 std::list<int> fds;
		 while (--cnt > 0) {
			 struct sockaddr_in6 _addr ;
			 memset(&_addr,0,sizeof(_addr));
			 _addr.sin6_family = AF_INET6;
			 _addr.sin6_port = htonl(PORT);
			 inet_pton(AF_INET6,"::1",&(_addr.sin6_addr));

			 int s = socket(AF_INET6,SOCK_STREAM,0);

			 ASSERT_TRUE(connect(s,(struct sockaddr*)&_addr,sizeof(_addr))==0);

			 ASSERT_TRUE(write(s,"Clifford",strlen("Clifford")+1)>0);
			 char b[1000];
			 ssize_t len = -1;

			 ASSERT_TRUE((len=read(s,b,sizeof(b)))>0);

			 printf("Received :%s\n",b);

			 fds.push_back(s);

			 while (fds.size()>5) {
				 s = *fds.begin();
				 fds.pop_front();
				 close(s);
			 }
			 std_usleep(500*1000);
		 }
	 });
	 ASSERT_TRUE(listen(s,10)==0);

	 event_params params;
	 params._event_base = event_base_new();

	 int flags = fcntl(s,F_GETFL);
	 if (flags>=0) flags |= O_NONBLOCK;
	 fcntl(s,F_SETFL,flags);

	 params._events["ev_accept"]= event_new(params._event_base,s,EV_READ|EV_PERSIST,on_accept ,&params);

	 event_add(params._events["ev_accept"],NULL);

	 mutex.unlock();

	 while(true) {
		 //event_base_dispatch(evt);
	 }

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
