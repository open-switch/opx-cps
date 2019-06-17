/*
 * Copyright (c) 2019 Dell Inc.
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


#include "cps_api_operation_debug.h"

#include "std_assert.h"

#include "event_log.h"

#include <string>
#include <vector>

void cps_api_object_log(cps_api_object_t obj, const char *prefix) {
	static const size_t DEF_LOG_LEN=10000;
	std::vector<char> buff(DEF_LOG_LEN);
	std::string _obj = cps_api_object_to_string(obj,&buff[0],buff.size());
	std::string _key = cps_api_key_print(cps_api_object_key(obj),&buff[0],buff.size());

	EV_LOG(INFO,DSAPI,0,"CPS-GET-LOG","key %s, data %s",_obj.c_str(),_key.c_str());
}

void cps_api_object_list_log(cps_api_object_list_t lst, const char *prefix) {
	for ( size_t ix = 0, mx = cps_api_object_list_size(lst) ; ix < mx ; ++ix ) {
		cps_api_object_t o = cps_api_object_list_get(lst,ix);
		STD_ASSERT(o!=nullptr);
		cps_api_object_log(o,prefix);
	}
}

void cps_api_get_request_log(cps_api_get_params_t *req) {
	EV_LOG(INFO,DSAPI,0,"CPS-GET-LOG","%d keys, &d filters, %d objects, timeout %d",
			(int)req->key_count,(int)cps_api_object_list_size(req->filters), cps_api_object_list_size(req->list),(int)req->timeout);
	cps_api_object_list_log(req->filters,"filters");
	cps_api_object_list_log(req->list,"objects");
}

void cps_api_commit_request_log(cps_api_transaction_params_t *req) {
	EV_LOG(INFO,DSAPI,0,"CPS-SET-LOG","change objs %d, revert-data %d, timeout %d",
			(int)cps_api_object_list_size(req->change_list),(int)cps_api_object_list_size(req->prev), (int)req->timeout);
	cps_api_object_list_log(req->change_list,"changes");
	cps_api_object_list_log(req->prev,"revert-data");

}
