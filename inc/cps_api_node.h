/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */
#ifndef CPS_API_NODE_H_
#define CPS_API_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cps_api_object.h"
#include "cps_api_errors.h"

#include <stddef.h>

typedef enum {
    cps_api_node_data_NODAL,                 /// Data resides on each node and is not duplicated by node group
    cps_api_node_data_1_PLUS_1_REDUNDENCY    /// Data is stored in a global database and replicated to a backup node
} cps_api_node_data_type_t;

typedef struct {
    const char * node_name;
    const char * addr;
} cps_api_node_ident;

typedef struct {
    const char * id;                 /// the group ID string.
    cps_api_node_ident *addrs;     /// the list of addresses for the nodes
    size_t addr_len;                ///the length of the addresses in this node
    cps_api_node_data_type_t data_type;        ///in the case of database clustering, determine if it will be 1+1 or nodal
} cps_api_node_group_t;


/**
 * Create a node grouping which allowing access to fanout requests to all of the addresses specified
 * @param group the group configuration
 * @return on success the call returns cps_api_ret_code_OK otherwise there will be a specific cps return code
 */
cps_api_return_code_t cps_api_set_node_group(cps_api_node_group_t *group);

/**
 * Delete a node grouping using the address specified.
 * @param group the name of the group to delete
 * @return on success the call returns cps_api_ret_code_OK otherwise there will be a specific cps return code
 */
cps_api_return_code_t cps_api_delete_node_group(const char *group);

/**
 * Create a list of aliases that when seen, will be converted to name - this is really focused on the IP/port combinations for the ipaddresses
 *     in a group.  This allows remapping or simplified aliases (eg.. IP address/port of Node 1 is 49.10.2.32:2342 and the software
 *     wants to use that address any time "127.0.0.1:1233" or "joe" or "jane" is seen
 *
 * @param name the string that will replace any of the aliases seen
 * @param alias the alias list that will be converted to "name" if seen
 * @param len the length of the alias list
 * @return on success the call returns cps_api_ret_code_OK otherwise there will be a specific cps return code
 */
cps_api_return_code_t cps_api_set_identity(const char *name, const char **alias, size_t len);


/**
 * Set the group attribute on the object to the string specified.  This will trigger the CPS to look up the node chracteristics based
 *     on the group specified.
 * @param obj the object in question
 * @param group the group name
 * @return true if the group has been set on the object otherwise false
 */
bool cps_api_key_set_group(cps_api_object_t obj,const char *group);

/**
 * Remove the node group and node name attribute on the object.
 * This will ensure that the object has no node group or node name references
 * @param obj the object in question
 */
void cps_api_key_del_node_attrs(cps_api_object_t obj);

/**
 * Get the string for the group attribute in the object specified.
 * @param obj the object in question
 * @return the string group ID
 */
const char * cps_api_key_get_group(cps_api_object_t obj);

/**
 * Get the node identifier from the object if it exists.
 * @param obj the object that may contain the node identifier
 * @return the character pointer for the object
 */
const char * cps_api_key_get_node(cps_api_object_t obj);


/**
 * Set the node identifier on the object
 * @param obj the object that may contain the node identifier
 * @param node is the name of the node
 * @return true if successful otherwise false
 */
bool cps_api_key_set_node(cps_api_object_t obj, const char *node);

/**
 * Set the master node of a group for 1+1 type group
 * @param group name of the group
 * @node_name name of the node which is part of the group
 * @return on success the call returns cps_api_ret_code_OK otherwise there will be a specific cps return code
 */
cps_api_return_code_t cps_api_set_master_node(const char *group,const char * node_name);

#ifdef __cplusplus
}
#endif

#endif /* CPS_API_NODE_H_ */
