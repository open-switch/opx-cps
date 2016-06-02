
#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_


#include <functional>
#include <string>

bool cps_api_node_set_iterate(const std::string &group_name,const std::function<void (const std::string &node, void*context)> &operation,
        void *context);


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_ */
