/*
 * cps_api_utils.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: cwichmann
 */

#include "private/cps_api_client_utils.h"

#include "std_user_perm.h"
#include "event_log.h"

#define CPS_USER_ID "cpsuser"
#define CPS_USER_GRPUP "cpsusers"

void cps_api_set_cps_file_perms(const char *path) {
    if (std_user_chmod(path,"o-rwxg+wr")!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPSAPI-FILE-PERM","Failed to set %s to to o-rwxg+ew",path);
    }
    if (std_user_chown(path,CPS_USER_ID,CPS_USER_GRPUP)!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPSAPI-FILE-PERM","Failed to set file %s to user %s",path,
                CPS_USER_ID);
    }
}
