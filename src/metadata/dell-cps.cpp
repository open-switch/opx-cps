
/*
* Copyright (c) 2017 Dell Inc.
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
dell-cps
*/
#include "dell-cps.h"
#include "cps_class_map.h"

static const struct {
  cps_api_attr_id_t ids[6]; //maximum of any keys in this file
  size_t ids_size;
  cps_api_attr_id_t id;
<<<<<<< HEAD
||||||| merged common ancestors
  cps_class_map_node_details details;
} lst[] = {
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_TYPE }, CPS_NODE_GROUP_TYPE, { "cps/node-group/type", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_RETURN_CODE }, CPS_OBJECT_GROUP_RETURN_CODE, { "cps/object-group/return-code", "In the event that an API needs to store a return code within an object itself, they can use this field.  This field isleft as an int size.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_NAME }, CPS_CONNECTION_ENTRY_NAME, { "cps/connection-entry/name", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_NAME }, CPS_NODE_DETAILS_NAME, { "cps/node-details/name", "The name to use as a replacement for any of the alias attributes.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP }, CPS_OBJECT_GROUP_GROUP, { "cps/object-group/group", "This attribute holds the group ID if specified.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_IP }, CPS_CONNECTION_ENTRY_IP, { "cps/connection-entry/ip", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_TUNNEL,CPS_TUNNEL_GROUP,CPS_TUNNEL_NODE_ID,CPS_TUNNEL_IP }, CPS_TUNNEL_IP, { "cps/tunnel/ip", "Ip address of the node.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME }, CPS_NODE_GROUP, { "cps/node-group", "This object is used to describe the mapping of a group ID to a list of node IP/port combinations.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_TUNNEL_IP }, CPS_NODE_GROUP_NODE_TUNNEL_IP, { "cps/node-group/node/tunnel-ip", "The IP addres and port of the stunnel client. Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TRANSACTION }, CPS_OBJECT_GROUP_TRANSACTION, { "cps/object-group/transaction", "In the case of commit operations (and the object is cacheable and doesn't exist at the timedue to a restart, then an Inprogress return code will be provided and this transaction IDcan be used to monitor the state of the transaction.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }},
{ { cps_api_obj_CAT_CPS,CPS_TUNNEL,CPS_TUNNEL_GROUP,CPS_TUNNEL_NODE_ID,CPS_TUNNEL_GROUP }, CPS_TUNNEL_GROUP, { "cps/tunnel/group", "Group name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_TUNNEL,CPS_TUNNEL_GROUP,CPS_TUNNEL_NODE_ID }, CPS_TUNNEL, { "cps/tunnel", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GET_NEXT }, CPS_OBJECT_GROUP_GET_NEXT, { "cps/object-group/get-next", "If this attribute is present in an object, then the caller would like the next object in lexicographic order", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_ALIAS }, CPS_NODE_DETAILS_ALIAS, { "cps/node-details/alias", "A list of alias that can be convered back to the name.", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE }, CPS_NODE_GROUP_NODE, { "cps/node-group/node", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_GROUP }, CPS_CONNECTION_ENTRY_GROUP, { "cps/connection-entry/group", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES }, CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES, { "cps/object-group/number-of-entries", "Provide the number of entries to retrieve at one time to support the concept of range along with the get next attribute", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }},
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID }, CPS_DB_INSTANCE, { "cps/db-instance", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_TUNNEL,CPS_TUNNEL_GROUP,CPS_TUNNEL_NODE_ID,CPS_TUNNEL_PORT }, CPS_TUNNEL_PORT, { "cps/tunnel/port", "System port where tunneling from DB to other node occurs.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_TUNNEL,CPS_TUNNEL_GROUP,CPS_TUNNEL_NODE_ID,CPS_TUNNEL_NODE_ID }, CPS_TUNNEL_NODE_ID, { "cps/tunnel/node-id", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME }, CPS_NODE_DETAILS, { "cps/node-details", "This object contains a list of aliases for a node.  Any one of the aliases will be convertedinto the name.For instance lets asume that there was a node with an IP address of 10.11.11.11 andthere was a port 443 that contained a service. The name would be 10.11.11.11:443 while on that node,lets say we wanted a user friendly name and therefore called the entity joe or jane but when you seejoe or jain, you want to convert these aliases for the node back to 10.11.11.11:443.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_NODE_ID }, CPS_DB_INSTANCE_NODE_ID, { "cps/db-instance/node-id", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP }, CPS_OBJECT_GROUP, { "cps/object-group", "These attributes are placed in objects by CPS infrastructure and provide additional information or changebehaviour as needed.", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_IP }, CPS_NODE_GROUP_NODE_IP, { "cps/node-group/node/ip", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NODE }, CPS_OBJECT_GROUP_NODE, { "cps/object-group/node", "This attribute contains the node id.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME }, CPS_CONNECTION_ENTRY, { "cps/connection-entry", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_CONNECTION_STATE }, CPS_CONNECTION_ENTRY_CONNECTION_STATE, { "cps/connection-entry/connection-state", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_PORT }, CPS_DB_INSTANCE_PORT, { "cps/db-instance/port", "System port where DB Instance was started for given group", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_EXACT_MATCH }, CPS_OBJECT_GROUP_EXACT_MATCH, { "cps/object-group/exact-match", "If this attribute is present in an event registration or query - then the attributes in the object will be used to detminematching events or objects.  To be more explicit on events and object queries:) in the case of events - only events matching the exact object key or attributes in the object will be provided to the application.) in the case of queries - if exact match is present, then the value of the exact match shall be used to determine if a wildcard matching behaviour will be used. If this value is not present, the wildcard matching will be guessed.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_NAME }, CPS_NODE_GROUP_NODE_NAME, { "cps/node-group/node/name", "The name of the node entry. (an alias for the ip)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NAME }, CPS_NODE_GROUP_NAME, { "cps/node-group/name", "The name of the group.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_FAILED_NODES }, CPS_OBJECT_GROUP_FAILED_NODES, { "cps/object-group/failed-nodes", "This attribute is used in an object to indicate which nodes did not process the request due toconnectivity reasons. Nodes are in a single string separated by comma (,).", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS }, cps_api_obj_CAT_CPS, { "cps", "", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_GROUP }, CPS_DB_INSTANCE_GROUP, { "cps/db-instance/group", "Group name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
};
=======
} _keys[] = {
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NAME }, 4, CPS_CONNECTIVITY_GROUP_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_TYPE }, 4, CPS_NODE_GROUP_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_CONFIG_TYPE }, 3, CPS_OBJECT_GROUP_CONFIG_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_TUNNEL_IP }, 5, CPS_CONNECTIVITY_GROUP_NODE_TUNNEL_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME }, 3, CPS_SERVICE_INSTANCE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_RETURN_CODE }, 3, CPS_OBJECT_GROUP_RETURN_CODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_NAME }, 4, CPS_CONNECTION_ENTRY_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_NAME }, 4, CPS_NODE_DETAILS_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP }, 3, CPS_OBJECT_GROUP_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_TIMESTAMP }, 5, CPS_CONNECTIVITY_GROUP_NODE_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_TUNNEL }, 4, CPS_CONNECTION_OBJECT_TUNNEL}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_IP }, 5, CPS_CONNECTIVITY_GROUP_NODE_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TRANSACTION }, 3, CPS_OBJECT_GROUP_TRANSACTION}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_IP }, 4, CPS_CONNECTION_ENTRY_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_ALIAS }, 4, CPS_NODE_DETAILS_ALIAS}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TIMESTAMP }, 3, CPS_OBJECT_GROUP_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_NAME }, 5, CPS_NODE_GROUP_NODE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_REGISTERED_KEY }, 4, CPS_SERVICE_INSTANCE_REGISTERED_KEY}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_NAME }, 5, CPS_CONNECTIVITY_GROUP_NODE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_SEQUENCE }, 3, CPS_OBJECT_GROUP_SEQUENCE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_WILDCARD_SEARCH }, 3, CPS_OBJECT_GROUP_WILDCARD_SEARCH}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME }, 3, CPS_CONNECTIVITY_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME }, 3, CPS_NODE_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GET_NEXT }, 3, CPS_OBJECT_GROUP_GET_NEXT}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_ADDR }, 4, CPS_CONNECTION_OBJECT_ADDR}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_FAILED_NODES }, 3, CPS_OBJECT_GROUP_FAILED_NODES}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE }, 4, CPS_NODE_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_GROUP }, 4, CPS_CONNECTION_ENTRY_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES }, 3, CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID }, 4, CPS_DB_INSTANCE}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_CONNECTION_INFORMATION }, 4, CPS_SERVICE_INSTANCE_CONNECTION_INFORMATION}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME }, 3, CPS_NODE_DETAILS}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_THREAD_ID }, 3, CPS_OBJECT_GROUP_THREAD_ID}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_NODE_ID }, 5, CPS_DB_INSTANCE_NODE_ID}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME }, 3, CPS_CONNECTION_ENTRY}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME }, 3, CPS_CONNECTION_OBJECT}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NODE }, 3, CPS_OBJECT_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_TIMESTAMP }, 4, CPS_CONNECTION_OBJECT_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP }, 2, CPS_OBJECT_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_CONNECTION_STATE }, 4, CPS_CONNECTION_ENTRY_CONNECTION_STATE}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_PORT }, 5, CPS_DB_INSTANCE_PORT}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_NAME }, 4, CPS_CONNECTION_OBJECT_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_EXACT_MATCH }, 3, CPS_OBJECT_GROUP_EXACT_MATCH}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_RETURN_STRING }, 3, CPS_OBJECT_GROUP_RETURN_STRING}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_NAME }, 4, CPS_SERVICE_INSTANCE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NAME }, 4, CPS_NODE_GROUP_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE }, 4, CPS_CONNECTIVITY_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_TYPE }, 4, CPS_CONNECTIVITY_GROUP_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_IP }, 5, CPS_NODE_GROUP_NODE_IP}, 
{ { cps_api_obj_CAT_CPS }, 1, cps_api_obj_CAT_CPS}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_TIMESTAMP }, 4, CPS_CONNECTIVITY_GROUP_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_GROUP }, 5, CPS_DB_INSTANCE_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE }, 3, CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE}, 
};
>>>>>>> integration

<<<<<<< HEAD
} _keys[] = {
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NAME }, 4, CPS_CONNECTIVITY_GROUP_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_TYPE }, 4, CPS_NODE_GROUP_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_CONFIG_TYPE }, 3, CPS_OBJECT_GROUP_CONFIG_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_TUNNEL_IP }, 5, CPS_CONNECTIVITY_GROUP_NODE_TUNNEL_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME }, 3, CPS_SERVICE_INSTANCE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_RETURN_CODE }, 3, CPS_OBJECT_GROUP_RETURN_CODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_NAME }, 4, CPS_CONNECTION_ENTRY_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_NAME }, 4, CPS_NODE_DETAILS_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP }, 3, CPS_OBJECT_GROUP_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_TIMESTAMP }, 5, CPS_CONNECTIVITY_GROUP_NODE_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_TUNNEL }, 4, CPS_CONNECTION_OBJECT_TUNNEL}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_IP }, 5, CPS_CONNECTIVITY_GROUP_NODE_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TRANSACTION }, 3, CPS_OBJECT_GROUP_TRANSACTION}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_IP }, 4, CPS_CONNECTION_ENTRY_IP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_ALIAS }, 4, CPS_NODE_DETAILS_ALIAS}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TIMESTAMP }, 3, CPS_OBJECT_GROUP_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_NAME }, 5, CPS_NODE_GROUP_NODE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_REGISTERED_KEY }, 4, CPS_SERVICE_INSTANCE_REGISTERED_KEY}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE,CPS_CONNECTIVITY_GROUP_NODE_NAME }, 5, CPS_CONNECTIVITY_GROUP_NODE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_SEQUENCE }, 3, CPS_OBJECT_GROUP_SEQUENCE}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_WILDCARD_SEARCH }, 3, CPS_OBJECT_GROUP_WILDCARD_SEARCH}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME }, 3, CPS_CONNECTIVITY_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME }, 3, CPS_NODE_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GET_NEXT }, 3, CPS_OBJECT_GROUP_GET_NEXT}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_ADDR }, 4, CPS_CONNECTION_OBJECT_ADDR}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_FAILED_NODES }, 3, CPS_OBJECT_GROUP_FAILED_NODES}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE }, 4, CPS_NODE_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_GROUP }, 4, CPS_CONNECTION_ENTRY_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES }, 3, CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID }, 4, CPS_DB_INSTANCE}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_CONNECTION_INFORMATION }, 4, CPS_SERVICE_INSTANCE_CONNECTION_INFORMATION}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME }, 3, CPS_NODE_DETAILS}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_THREAD_ID }, 3, CPS_OBJECT_GROUP_THREAD_ID}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_NODE_ID }, 5, CPS_DB_INSTANCE_NODE_ID}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME }, 3, CPS_CONNECTION_ENTRY}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME }, 3, CPS_CONNECTION_OBJECT}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NODE }, 3, CPS_OBJECT_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_TIMESTAMP }, 4, CPS_CONNECTION_OBJECT_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP }, 2, CPS_OBJECT_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_ENTRY,CPS_CONNECTION_ENTRY_NAME,CPS_CONNECTION_ENTRY_CONNECTION_STATE }, 4, CPS_CONNECTION_ENTRY_CONNECTION_STATE}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_PORT }, 5, CPS_DB_INSTANCE_PORT}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTION_OBJECT,CPS_CONNECTION_OBJECT_NAME,CPS_CONNECTION_OBJECT_NAME }, 4, CPS_CONNECTION_OBJECT_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_EXACT_MATCH }, 3, CPS_OBJECT_GROUP_EXACT_MATCH}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_RETURN_STRING }, 3, CPS_OBJECT_GROUP_RETURN_STRING}, 
{ { cps_api_obj_CAT_CPS,CPS_SERVICE_INSTANCE,CPS_SERVICE_INSTANCE_NAME,CPS_SERVICE_INSTANCE_NAME }, 4, CPS_SERVICE_INSTANCE_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NAME }, 4, CPS_NODE_GROUP_NAME}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_NODE }, 4, CPS_CONNECTIVITY_GROUP_NODE}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_TYPE }, 4, CPS_CONNECTIVITY_GROUP_TYPE}, 
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NODE,CPS_NODE_GROUP_NODE_IP }, 5, CPS_NODE_GROUP_NODE_IP}, 
{ { cps_api_obj_CAT_CPS }, 1, cps_api_obj_CAT_CPS}, 
{ { cps_api_obj_CAT_CPS,CPS_CONNECTIVITY_GROUP,CPS_CONNECTIVITY_GROUP_NAME,CPS_CONNECTIVITY_GROUP_TIMESTAMP }, 4, CPS_CONNECTIVITY_GROUP_TIMESTAMP}, 
{ { cps_api_obj_CAT_CPS,CPS_DB_INSTANCE,CPS_DB_INSTANCE_GROUP,CPS_DB_INSTANCE_NODE_ID,CPS_DB_INSTANCE_GROUP }, 5, CPS_DB_INSTANCE_GROUP}, 
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE }, 3, CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE}, 
 };
 
static const cps_class_map_node_details _details[/*53*/] = {
{ "cps/connectivity-group/name", "The name of the group.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/type", "Type of the group cluster", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/object-group/config-type", "This attribute indicates the storage options for the object if supported.  This storage type can be1) Store to running2) Store to statup 3) Store to both running and startup", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/connectivity-group/node/tunnel-ip", "The IP addres and port of the stunnel client. Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance", "This service is registered by the CPS infra and can be used to provide details about active registrations in the system.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/return-code", "In the event that an API needs to store a return code within an object itself, they can use this field.  This field isleft as an int size.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 },
{ "cps/connection-entry/name", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details/name", "The name to use as a replacement for any of the alias attributes.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/group", "This attribute holds the group ID if specified.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/timestamp", "Timestamp when the node has connectivity established", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/tunnel", "System port where tunneling from DB to other node occurs", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/ip", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/transaction", "In the case of commit operations (and the object is cacheable and doesn't exist at the timedue to a restart, then an Inprogress return code will be provided and this transaction IDcan be used to monitor the state of the transaction.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/connection-entry/ip", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details/alias", "A list of alias that can be convered back to the name.", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/timestamp", "This is the timestamp of when the event is generated.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/node-group/node/name", "The name of the node entry. (an alias for the ip)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance/registered-key", "The registered keys that are associated with the service.", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/name", "The name of the node entry. (an alias for the ip)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/sequence", "This field will hold a CPS internal sequence number.  This number will be used to determine if duplicate events are being sent andto help track out of order events.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/object-group/wildcard-search", "This attribute is true if the attributes in the object (primarily used for get requests) contain wildcard characters.For instance if someone was searching for a python interface/vlan object with all names starting with eth, the python dictionarywould appear as follows:{'key': 'interface/vlan','data' : { 'interface/vlan/name' : 'eth*',      'cps/object-group/wildcard-search': True }}", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/connectivity-group", "This object is used to describe the mapping of a group ID to a list of node IP/port combinations.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/node-group", "This object is used to describe the mapping of a group ID to a list of node IP/port combinations.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/get-next", "If this attribute is present in an object, then the caller would like the next object in lexicographic order", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/connection-object/addr", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/failed-nodes", "This attribute is used in an object to indicate which nodes did not process the request due toconnectivity reasons. Nodes are in a single string separated by comma (,).", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/node", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-entry/group", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/number-of-entries", "Provide the number of entries to retrieve at one time to support the concept of range along with the get next attribute", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/db-instance", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/service-instance/connection-information", "Protocol specific connection information.  For instance, in the case of a UNIX domain socket connection this is the socket name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details", "This object contains a list of aliases for a node.  Any one of the aliases will be convertedinto the name.For instance lets asume that there was a node with an IP address of 10.11.11.11 andthere was a port 443 that contained a service. The name would be 10.11.11.11:443 while on that node,lets say we wanted a user friendly name and therefore called the entity joe or jane but when you seejoe or jain, you want to convert these aliases for the node back to 10.11.11.11:443.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/thread-id", "The thread ID of the person making the request or sending the event.This is used internally for debugging purposes only.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/db-instance/node-id", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-entry", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-object", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/node", "This attribute contains the node id.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/timestamp", "Timestamp when the node has connectivity established", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group", "These attributes are placed in objects by CPS infrastructure and provide additional information or changebehaviour as needed.", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-entry/connection-state", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/db-instance/port", "System port where DB Instance was started for given group", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/name", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/exact-match", "If this attribute is present in an filter object, then the attributes in the object will be used to findobjects who's attributes match the attribute values specified in this filter object.For examplpe:A { 1=A,2=B } will match B { 1=A,2=B,C=3,D=4 }A { 1=A,2=B } will not match { 1=A,C=3 }", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/object-group/return-string", "This field can be used to store a string description (tag) of either an error that occurued during the requested operation or additionalinformation to the successful request (less likely).", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance/name", "The service name or some type of string representation of the service instance.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/name", "The name of the group.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connectivity-group/type", "Type of the group cluster", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/node-group/node/ip", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps", "", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connectivity-group/timestamp", "Timestamp when the group was created", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/db-instance/group", "Group name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/continue-on-failure", "This attribute if provided will control the behaviour of CPS in error conditions for both the gets and commits.In the case of a commit, if this attribute is present and false, then a rollback will not be performed and the operation will continue.  Errors discovered will be returned within the objects themselves.    		In the case of a get, if this attribute is present on the filter, all errors in gets will be ignored as welland in this case, objects will not be returned.    		Eg..1 commit transaction with 3 objects and one object fails (with this flag set to false) the CPS layer will continue to perform all three commits and will not rollback.    		Eg..1 in the get case, with 3 object filters, if there is no results or a failure in the backend component,CPS will ignore the error and continue.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
};
||||||| merged common ancestors
=======
static const cps_class_map_node_details _details[/*53*/] = {
{ "cps/connectivity-group/name", "The name of the group.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/type", "Type of the group cluster", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/object-group/config-type", "This attribute indicates the storage options for the object if supported.  This storage type can be1) Store to running2) Store to statup 3) Store to both running and startup", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/connectivity-group/node/tunnel-ip", "The IP addres and port of the stunnel client. Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance", "This service is registered by the CPS infra and can be used to provide details about active registrations in the system.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/return-code", "In the event that an API needs to store a return code within an object itself, they can use this field.  This field isleft as an int size.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 },
{ "cps/connection-entry/name", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details/name", "The name to use as a replacement for any of the alias attributes.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/group", "This attribute holds the group ID if specified.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/timestamp", "Timestamp when the node has connectivity established", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/tunnel", "System port where tunneling from DB to other node occurs", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/ip", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/transaction", "In the case of commit operations (and the object is cacheable and doesn't exist at the timedue to a restart, then an Inprogress return code will be provided and this transaction IDcan be used to monitor the state of the transaction.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/connection-entry/ip", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details/alias", "A list of alias that can be convered back to the name.", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/timestamp", "This is the timestamp of when the event is generated.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/node-group/node/name", "The name of the node entry. (an alias for the ip)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance/registered-key", "The registered keys that are associated with the service.", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node/name", "The name of the node entry. (an alias for the ip)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/sequence", "This field will hold a CPS internal sequence number.  This number will be used to determine if duplicate events are being sent andto help track out of order events.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/object-group/wildcard-search", "This attribute is true if the attributes in the object (primarily used for get requests) contain wildcard characters.For instance if someone was searching for a python interface/vlan object with all names starting with eth, the python dictionarywould appear as follows:{'key': 'interface/vlan','data' : { 'interface/vlan/name' : 'eth*',      'cps/object-group/wildcard-search': True }}", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/connectivity-group", "This object is used to describe the mapping of a group ID to a list of node IP/port combinations.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/node-group", "This object is used to describe the mapping of a group ID to a list of node IP/port combinations.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/get-next", "If this attribute is present in an object, then the caller would like the next object in lexicographic order", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/connection-object/addr", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/failed-nodes", "This attribute is used in an object to indicate which nodes did not process the request due toconnectivity reasons. Nodes are in a single string separated by comma (,).", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/node", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-entry/group", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/number-of-entries", "Provide the number of entries to retrieve at one time to support the concept of range along with the get next attribute", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/db-instance", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/service-instance/connection-information", "Protocol specific connection information.  For instance, in the case of a UNIX domain socket connection this is the socket name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-details", "This object contains a list of aliases for a node.  Any one of the aliases will be convertedinto the name.For instance lets asume that there was a node with an IP address of 10.11.11.11 andthere was a port 443 that contained a service. The name would be 10.11.11.11:443 while on that node,lets say we wanted a user friendly name and therefore called the entity joe or jane but when you seejoe or jain, you want to convert these aliases for the node back to 10.11.11.11:443.", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/thread-id", "The thread ID of the person making the request or sending the event.This is used internally for debugging purposes only.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 },
{ "cps/db-instance/node-id", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-entry", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-object", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/object-group/node", "This attribute contains the node id.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/timestamp", "Timestamp when the node has connectivity established", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group", "These attributes are placed in objects by CPS infrastructure and provide additional information or changebehaviour as needed.", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connection-entry/connection-state", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/db-instance/port", "System port where DB Instance was started for given group", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connection-object/name", "Node name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/exact-match", "If this attribute is present in an filter object, then the attributes in the object will be used to findobjects who's attributes match the attribute values specified in this filter object.For examplpe:A { 1=A,2=B } will match B { 1=A,2=B,C=3,D=4 }A { 1=A,2=B } will not match { 1=A,C=3 }", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
{ "cps/object-group/return-string", "This field can be used to store a string description (tag) of either an error that occurued during the requested operation or additionalinformation to the successful request (less likely).", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/service-instance/name", "The service name or some type of string representation of the service instance.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/node-group/name", "The name of the group.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/connectivity-group/node", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connectivity-group/type", "Type of the group cluster", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_ENUM },
{ "cps/node-group/node/ip", "The IP address and port of the element.  Valid IP address/portcombinations are IPv4:port or IPv6:port", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps", "", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN },
{ "cps/connectivity-group/timestamp", "Timestamp when the group was created", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/db-instance/group", "Group name", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING },
{ "cps/object-group/continue-on-failure", "This attribute if provided will control the behaviour of CPS in error conditions for both the gets and commits.In the case of a commit, if this attribute is present and false, then a rollback will not be performed and the operation will continue.  Errors discovered will be returned within the objects themselves.    		In the case of a get, if this attribute is present on the filter, all errors in gets will be ignored as welland in this case, objects will not be returned.    		Eg..1 commit transaction with 3 objects and one object fails (with this flag set to false) the CPS layer will continue to perform all three commits and will not rollback.    		Eg..1 in the get case, with 3 object filters, if there is no results or a failure in the backend component,CPS will ignore the error and continue.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL },
};
>>>>>>> integration

<<<<<<< HEAD
static const size_t lst_len = sizeof(_keys)/sizeof(*_keys);
extern "C"{ 
  t_std_error cps_api_yang_module_init(void) {
||||||| merged common ancestors
static const size_t lst_len = sizeof(lst)/sizeof(*lst);

t_std_error cps_api_yang_module_init(void) {
=======

static const size_t lst_len = sizeof(_keys)/sizeof(*_keys);
extern "C"{ 
  t_std_error cps_api_yang_module_init(void) {
>>>>>>> integration
    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) { 
        cps_class_map_init(_keys[ix].id,_keys[ix].ids,_keys[ix].ids_size,_details+ix); 
    }
    return STD_ERR_OK;
  }
}
