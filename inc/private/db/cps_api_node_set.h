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

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_


#include <functional>
#include <string>

bool cps_api_node_set_iterate(const std::string &group_name,const std::function<void (const std::string &node, void*context)> &operation,
        void *context);


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_NODE_SET_H_ */
