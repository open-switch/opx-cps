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

#ifndef CPS_API_INC_PRIVATE_CPS_API_KEY_UTILS_H_
#define CPS_API_INC_PRIVATE_CPS_API_KEY_UTILS_H_

#include "cps_api_object.h"
#include "cps_api_key.h"

#include <stddef.h>
#include <functional>

size_t cps_api_object_count_key_attrs(cps_api_object_t obj);

void cps_api_object_iterate_key_attrs(cps_api_object_t obj, std::function<void(cps_api_object_t,
								cps_api_attr_id_t,void *,size_t)> &iter);


uint64_t cps_api_hash_array(const cps_api_key_element_t *data, size_t len);

int cps_api_key_array_matches(const cps_api_key_element_t *lhs, size_t lhs_size,
		const cps_api_key_element_t *rhs, size_t rhs_size, bool exact) ;


#endif /* CPS_API_INC_PRIVATE_CPS_API_KEY_UTILS_H_ */
