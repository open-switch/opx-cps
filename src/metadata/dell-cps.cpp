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

/**
 * @file dell-cps.cpp
 */

#include "dell-cps.h"
#include "cps_class_map.h"

#include <vector>

static struct {
  std::vector<cps_api_attr_id_t> _ids;
  cps_api_attr_id_t id;
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


static const size_t lst_len = sizeof(lst)/sizeof(*lst);

t_std_error cps_api_yang_module_init(void) {
    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
    return STD_ERR_OK;
}

