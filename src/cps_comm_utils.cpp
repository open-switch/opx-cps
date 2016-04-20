/*
 * cps_comm_utils.cpp
 *
 *  Created on: Mar 26, 2016
 *      Author: cwichmann
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
