
/*
* source file : dell-base-cps.h
*/


/*
* Copyright (c) 2015 Dell Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may
* not use this file except in compliance with the License. You may obtain
* a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
* LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
* FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
* See the Apache Version 2.0 License for specific language governing
* permissions and limitations under the License.
*/
#ifndef DELL_BASE_CPS_H
#define DELL_BASE_CPS_H

#include "cps_api_operation.h"
#include "std_error_codes.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_CPS (2) 

#define DELL_BASE_CPS_MODEL_STR "dell-base-cps"


/* Object cps/node-details */

typedef enum { 

/*type=string*/ 
  CPS_NODE_DETAILS_NAME = 131073,

/*type=string*/ 
  CPS_NODE_DETAILS_ALIAS = 131081,
} CPS_NODE_DETAILS_t;
/* Object cps/object-group */

typedef enum { 

/*type=string*/ 
  CPS_OBJECT_GROUP_GROUP = 131077,

/*type=uint64*/ 
  CPS_OBJECT_GROUP_TRANSACTION = 131079,

/*type=string*/ 
  CPS_OBJECT_GROUP_NODE = 131080,
} CPS_OBJECT_GROUP_t;
/* Object cps/node-group */

typedef enum { 

/*type=string*/ 
  CPS_NODE_GROUP_NAME = 131082,

/*type=string*/ 
  CPS_NODE_GROUP_IP = 131083,

/*type=uint32*/ 
  CPS_NODE_GROUP_TYPE = 131084,
} CPS_NODE_GROUP_t;

/* Object's continued */

typedef enum{
  CPS_NODE_DETAILS = 131075,
  CPS_NODE_DETAILS_OBJ = 131075,

  CPS_NODE_GROUP = 131085,
  CPS_NODE_GROUP_OBJ = 131085,

  CPS_OBJECT_GROUP = 131078,
  CPS_OBJECT_GROUP_OBJ = 131078,

} CPS_OBJECTS_t;

#ifdef __cplusplus
extern "C" {
#endif

t_std_error cps_api_yang_module_init();

#ifdef __cplusplus
}
#endif

#endif
