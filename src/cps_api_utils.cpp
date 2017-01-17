/*
 * Copyright (c) 2016 Dell Inc.
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
 * cps_api_utils.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: cwichmann
 */

#include "private/cps_api_client_utils.h"

#include "std_user_perm.h"
#include "event_log.h"

#define CPS_USER_ID "_opx_cps"
#define CPS_USER_GRPUP "_opx_cps"

void cps_api_set_cps_file_perms(const char *path) {
    if (std_user_chmod(path,"o-rwx")!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPSAPI-FILE-PERM","Failed to set %s to to o-rwx",path);
    }
    if (std_user_chmod(path,"g+rwx")!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPSAPI-FILE-PERM","Failed to set %s to to o-rwx",path);
    }
    if (std_user_chown(path,CPS_USER_ID,CPS_USER_GRPUP)!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPSAPI-FILE-PERM","Failed to set file %s to user %s",path,
                CPS_USER_ID);
    }
}
