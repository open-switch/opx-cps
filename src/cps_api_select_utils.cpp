/*
 * cps_api_select_utils.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: cwichmann
 */

#include "cps_api_select_utils.h"
#include "cps_comm_utils.h"

#include <sys/epoll.h>
#include <mutex>
#include <list>
#include <unistd.h>
#include <memory>
#include <string.h>


struct __handle_struct {
	int _fd;
	cps_api_select_settings _settings;

	bool _select_needs_to_be_closed = false;
	epoll_event _epoll_op_defaults;

};

namespace {

std::list<__handle_struct*> __select_entries_cache;
std::mutex __mutex;
bool __shutdown = false;

}

bool cps_api_select_utils_init() {

	return true;
}
void cps_api_select_utils_close() {
}

void cps_api_select_dealloc(cps_api_select_handle_t h) {
	if (h==nullptr) return ;

	std::unique_ptr<__handle_struct> _h((__handle_struct*)h);
	if (_h->_select_needs_to_be_closed || __shutdown == true) {
		::close(_h->_fd);
		return;
	}
	__select_entries_cache.push_back(_h.get());
	_h.release();
	return;
}

namespace {

void cleanup(cps_api_select_handle_t h) {
	__handle_struct *p = (__handle_struct*)h;
	if (p->_fd!=-1) ::close(p->_fd);
	delete p;
}

}

cps_api_select_handle_t cps_api_select_alloc(const cps_api_select_settings &settings) {
	__handle_struct * _handle = nullptr;

	{
	std::lock_guard<std::mutex> _lg(__mutex);
	if (__select_entries_cache.size()>0) {
		_handle = __select_entries_cache.front();
		__select_entries_cache.pop_front();
	}
	}
	if (_handle==nullptr) {
		std::unique_ptr<__handle_struct> h(new __handle_struct);
		if (h.get()==nullptr) return nullptr;
		h->_fd = epoll_create(1);
		if (h->_fd==-1) return nullptr;
		_handle = h.release();
	}
	_handle->_settings = settings;
	_handle->_epoll_op_defaults.data.fd=0;
	_handle->_epoll_op_defaults.events = ((settings.read_true_write_false) ? EPOLLIN : EPOLLOUT);

	return _handle;
}

bool cps_api_select_add_fd(cps_api_select_handle_t handle,int fd) {
	__handle_struct *_h = (__handle_struct*)handle;
	epoll_event _settings = _h->_epoll_op_defaults;
	return (epoll_ctl(_h->_fd,EPOLL_CTL_ADD,fd,&_settings)>=0);
}

int cps_api_select_wait(cps_api_select_handle_t handle, int *handles, size_t len, size_t timeoutms) {
	__handle_struct *_h = (__handle_struct*)handle;
	epoll_event _events[len];
	if (timeoutms==0) timeoutms=-1;
	int _rc = epoll_wait(_h->_fd,_events,len,timeoutms) ;
	if (_rc>0) {
		for ( ssize_t ix = 0; ix < _rc ; ++ix) {
			handles[ix] = _events[ix].data.fd;
		}
	}
	return _rc;
}

void cps_api_select_remove_fd(cps_api_select_handle_t handle,int fd) {
	__handle_struct *_h = (__handle_struct*)handle;
	epoll_ctl(_h->_fd,EPOLL_CTL_DEL,fd,nullptr);
}

bool cps_api_select_guard::add_fd(int fd) {
	if (!cps_api_select_add_fd(__h,fd)) return false;
	__cleanup_fds.insert(fd);
	return true;
}
void cps_api_select_guard::remove_fd(int fd) {
	cps_api_select_remove_fd(__h,fd);
	__cleanup_fds.erase(fd);
}

ssize_t cps_api_select_guard::get_events(int *handles, size_t len, size_t timeout) {
	return cps_api_select_wait(__h,handles,len,timeout);
}

ssize_t cps_api_select_guard::get_event(size_t timeout,int *handle) {
	int _handle;
	if (handle==nullptr) handle = &_handle;
	return get_events(handle,1,timeout);
}

void cps_api_select_guard::close() {
	if (__h==nullptr) return;

	for (auto &it : __cleanup_fds) {
		cps_api_select_remove_fd(__h,it);
	}
	__cleanup_fds.clear();
	cps_api_select_dealloc(__h);

	__h=nullptr;
}
