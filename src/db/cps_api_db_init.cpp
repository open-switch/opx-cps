
#include "cps_api_node_private.h"

#include "cps_api_db.h"


cps_api_return_code_t cps_db::cps_api_db_init() {
	const char *addr = DEFAULT_REDIS_ADDR;
	cps_api_node_group_t _group;
	_group.id = CPS_API_NODE_LOCAL_GROUP;
	_group.addr_len=1;
	_group.addresses=&addr;
	_group.data_type = cps_api_node_data_NODAL;

	cps_api_create_node_group(&_group);

	return cps_api_ret_code_OK;
}
