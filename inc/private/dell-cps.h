
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
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_CPS (258)

#define DELL_BASE_CPS_MODEL_STR "dell-base-cps"


/* Object cps/node-group/node */

typedef enum {
/*The name of the node entry. (an alias for the ip)*/
/*type=string*/
  CPS_NODE_GROUP_NODE_NAME = 16908289,
/*The IP address and port of the element.  Valid IP address/port
combinations are IPv4:port or [IPv6]:port*/
/*type=string*/
  CPS_NODE_GROUP_NODE_IP = 16908290,
} CPS_NODE_GROUP_NODE_t;
/* Object cps/connection-entry */

typedef enum {

/*type=string*/
  CPS_CONNECTION_ENTRY_NAME = 16908291,

/*type=string*/
  CPS_CONNECTION_ENTRY_IP = 16908292,

/*type=string*/
  CPS_CONNECTION_ENTRY_GROUP = 16908293,

/*type=boolean*/
  CPS_CONNECTION_ENTRY_CONNECTION_STATE = 16908294,
} CPS_CONNECTION_ENTRY_t;
/* Object cps/db-instance */

typedef enum {
/*Group name*/
/*type=string*/
  CPS_DB_INSTANCE_GROUP = 16908295,
/*Node name*/
/*type=string*/
  CPS_DB_INSTANCE_NODE_ID = 16908313,
/*System port where DB Instance was started for given group*/
/*type=string*/
  CPS_DB_INSTANCE_PORT = 16908297,
} CPS_DB_INSTANCE_t;
/* Object cps/node-details */

typedef enum {
/*The name to use as a replacement for any of the alias attributes.*/
/*type=string*/
  CPS_NODE_DETAILS_NAME = 16908298,
/*A list of alias that can be convered back to the name.*/
/*type=string*/
  CPS_NODE_DETAILS_ALIAS = 16908299,
} CPS_NODE_DETAILS_t;
/* Object cps/object-group */

typedef enum {
/*This attribute holds the group ID if specified.*/
/*type=string*/
  CPS_OBJECT_GROUP_GROUP = 16908300,
/*This attribute contains the node id.*/
/*type=string*/
  CPS_OBJECT_GROUP_NODE = 16908301,
/*In the case of commit operations (and the object is cacheable and doesn't exist at the time
due to a restart, then an Inprogress return code will be provided and this transaction ID
can be used to monitor the state of the transaction.*/
/*type=uint64*/
  CPS_OBJECT_GROUP_TRANSACTION = 16908302,
/*This attribute is used in an object to indicate which nodes did not process the request due to
connectivity reasons. Nodes are in a single string separated by comma (,).*/
/*type=string*/
  CPS_OBJECT_GROUP_FAILED_NODES = 16908303,
} CPS_OBJECT_GROUP_t;
/* Object cps/node-group */

typedef enum {
/*The name of the group.*/
/*type=string*/
  CPS_NODE_GROUP_NAME = 16908304,

/*type=binary*/
  CPS_NODE_GROUP_NODE = 16908305,

/*type=uint32*/
  CPS_NODE_GROUP_TYPE = 16908306,
} CPS_NODE_GROUP_t;

/* Object's continued */

typedef enum{
/*This object contains a list of aliases for a node.  Any one of the aliases will be converted
into the name.

For instance lets asume that there was a node with an IP address of 10.11.11.11 and
there was a port 443 that contained a service. The name would be 10.11.11.11:443 while on that node,
lets say we wanted a user friendly name and therefore called the entity joe or jane but when you see
joe or jain, you want to convert these aliases for the node back to 10.11.11.11:443.*/
  CPS_NODE_DETAILS = 16908307,
  CPS_NODE_DETAILS_OBJ = 16908307,

/*This object is used to describe the mapping of a group ID to a list of node IP/port combinations.*/
  CPS_NODE_GROUP = 16908308,
  CPS_NODE_GROUP_OBJ = 16908308,

/*These attributes are placed in objects by CPS infrastructure.*/
  CPS_OBJECT_GROUP = 16908309,
  CPS_OBJECT_GROUP_OBJ = 16908309,

  CPS_CONNECTION_ENTRY = 16908310,
  CPS_CONNECTION_ENTRY_OBJ = 16908310,

  CPS_DB_INSTANCE = 16908311,
  CPS_DB_INSTANCE_OBJ = 16908311,

} CPS_OBJECTS_t;


#endif
