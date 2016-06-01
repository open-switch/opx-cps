#ifndef CPS_API_NODE_H_
#define CPS_API_NODE_H_

#include "cps_api_object.h"
#include "cps_api_errors.h"

#include <stddef.h>

typedef enum {
	cps_api_node_data_NODAL, 				/// Data resides on each node and is not duplicated by node group
	cps_api_node_data_1_PLUS_1_REDUNDENCY	/// Data is stored in a global database and replicated to a backup node
} cps_api_node_data_type_t;

struct cps_api_node_group_t {
	const char * id; 				/// the group ID string.
	const char **addresses;  /// the list of addresses for the nodes
	size_t addr_len;			///the length of the addresses in this node

	cps_api_node_data_type_t data_type;
};

#define CPS_API_NODE_LOCAL_GROUP "localhost"

cps_api_return_code_t cps_api_create_node_group(cps_api_node_group_t *group);

cps_api_return_code_t cps_api_update_node_group(cps_api_node_group_t *group);

cps_api_return_code_t cps_api_delete_node_group(const char *grp);

cps_api_return_code_t cps_api_set_identity(const char *name, const char **alias, size_t len);



bool cps_api_key_set_group(cps_api_object_t obj,const char *group);

const char * cps_api_key_get_group(cps_api_object_t obj);

const char * cps_api_key_get_node(cps_api_object_t obj);


#endif /* CPS_API_NODE_H_ */
