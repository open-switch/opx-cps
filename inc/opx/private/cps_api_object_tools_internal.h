/*
 * Copyright (c) 2018 Dell Inc.
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
 *
 *
 * cps_api_object_tools_internal.h
 *
 *  Created on: May 19, 2017
 */

#ifndef CPS_API_OBJECT_TOOLS_INTERNAL_H_
#define CPS_API_OBJECT_TOOLS_INTERNAL_H_

#include "cps_api_object.h"

static inline bool cps_api_obj_attr_get_bool(cps_api_object_t obj, cps_api_attr_id_t id, bool default_value=false) {
   const bool *_p = (const bool*)cps_api_object_get_data(obj,id);
	return _p==nullptr ? default_value : *_p;
}


#endif /* CPS_API_OBJECT_TOOLS_INTERNAL_H_ */
