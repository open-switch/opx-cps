/*
 * Copyright (c) 2018 Dell Inc.
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
 */

#include "cps_comm_utils.h"
#include "cps_string_utils.h"

#include <sys/syscall.h>
#include <inttypes.h>

#include <mutex>

#include <stdio.h>
#include <unistd.h>


namespace cps_comms {

std::string cps_generate_unique_process_key() {
    static std::mutex __lock;
    std::lock_guard<std::mutex> lg(__lock);
    static size_t __i =0;
    pid_t __pid = getpid();
    pid_t __tid = (pid_t) syscall (SYS_gettid);
    return cps_string::sprintf("%d-%d-%" PRIx64, __pid,__tid,__i++);
}


}
