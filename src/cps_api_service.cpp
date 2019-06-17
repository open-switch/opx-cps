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
 */

/*
 * cps_api_service.cpp
 */

#include "cps_api_service.h"
#include "private/cps_ns.h"
#include "private/db/cps_api_db.h"

#include "cps_class_map.h"
#include "std_event_service.h"
#include "event_log.h"
#include "std_assert.h"

#include <unistd.h>             /* pause() */
#include <signal.h>             /* signal(), SIGTERM */
#include <systemd/sd-daemon.h>  /* sd_notify() */
#include <systemd/sd-journal.h> /* sd_journal_print() */
#include <sys/resource.h>       /* setrlimit() */
#include <stdio.h>              /* setvbuf() */
#include <stdlib.h>             /* EXIT_SUCCESS */

static int signo_caught = -1;
static bool __running = true;
static void sigterm_handler(int signo) {
    signo_caught = signo;
    if (signo==SIGTERM) __running = false;
}

int main(int argc, char**argv) {
    EV_LOGGING(CPS,ERR,"SERVICE", "Entering CPS API Service");

    // Enable core dumps
    static const struct rlimit rlim = { .rlim_cur = ~0U, .rlim_max = ~0U };
    setrlimit(RLIMIT_CORE, &rlim);

    // Standard output is buffered. Any messages printed with printf()
    // will not go to the journal at the same time as standard error
    // or syslog messages. To ensure stdout messages show up at the
    // right time in the journal we need to diable stdout buffering.
    setvbuf(stdout, NULL, _IONBF, 0); // Unbuffered

    // Install SIGTERM handler. By default, systemd uses SIGTERM
    // to stop deamons (e.g. systemctl stop cps_api_svc.service)
    (void)signal(SIGTERM, sigterm_handler);

    //Preload the class meta data before the startup of the nameserver process
    cps_api_class_map_init();

    STD_ASSERT(cps_db::cps_api_db_init()==cps_api_ret_code_OK);

    if (cps_api_services_start()!=cps_api_ret_code_OK) {
        EV_LOG(ERR,DSAPI,0,"FLT","Failed to initialize the messaging service.");
        return cps_api_ret_code_ERR;
    }
    if (cps_api_ns_startup()!=cps_api_ret_code_OK) {
        return cps_api_ret_code_ERR;
    }

    EV_LOGGING(CPS,DEBUG,"SERVICE","Sending READY to systemd");
    sd_notify(0, "READY=1");

    EV_LOGGING(CPS,DEBUG,"SERVICE", "Entering main loop (i.e. wait forever for signals)");
    while (__running) {
        (void)pause(); // Wait for signals to be caught by sig_handler()
    }

    /* ADD SHUTDOWN CODE HERE (IF NEEDED) */

    EV_LOGGING(CPS,DEBUG,"SERVICE","Signal %d received - Shutting down now!",
                     signo_caught);

    exit(EXIT_SUCCESS);
}
