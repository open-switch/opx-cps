/*
 * Copyright (c) 2019 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 *
 * cps_api_select_utils.h
 *
 *  Created on: Dec 11, 2016
 */

#ifndef CPS_API_INC_PRIVATE_CPS_API_SELECT_UTILS_H_
#define CPS_API_INC_PRIVATE_CPS_API_SELECT_UTILS_H_

#include <unistd.h>
#include <unordered_set>

struct cps_api_select_settings {
    bool read_true_write_false=false;
    cps_api_select_settings() {}
    cps_api_select_settings(bool b) : read_true_write_false(b){}
};

using cps_api_select_handle_t = void *;

bool cps_api_select_utils_init();
void cps_api_select_utils_close();

cps_api_select_handle_t cps_api_select_alloc(const cps_api_select_settings &settings);

static inline cps_api_select_handle_t cps_api_select_alloc_read() {
    auto _s = cps_api_select_settings{true};
    return cps_api_select_alloc(_s);
}

void cps_api_select_dealloc(cps_api_select_handle_t handle);

bool cps_api_select_add_fd(cps_api_select_handle_t handle, int fd, bool *read, bool *write);

int cps_api_select_wait(cps_api_select_handle_t h, int *handles, size_t len, size_t timeoutms);
void cps_api_select_remove_fd(cps_api_select_handle_t handle, int fd);

class cps_api_select_guard{
    cps_api_select_handle_t __h;
    std::unordered_set<int> __cleanup_fds;
public:
    cps_api_select_guard(cps_api_select_handle_t h) : __h(h) {}

    bool valid() { return __h!=nullptr; }
    cps_api_select_handle_t get() { return __h; }

    bool add_fd(int fd, bool *read=nullptr,bool *write=nullptr);
    void remove_fd(int fd);

    void close();
    ~cps_api_select_guard() { close() ; }

    ssize_t get_events(int *handles, size_t len, size_t timeout);
    ssize_t get_event(size_t timeout, int * handle=nullptr);

    void remove_all_fds();
};

#endif /* CPS_API_INC_PRIVATE_CPS_API_SELECT_UTILS_H_ */
