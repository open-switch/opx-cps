/*
 * cps_api_key_utils.h
 *
 *  Created on: Sep 1, 2016
 *      Author: cwichmann
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

#endif /* CPS_API_INC_PRIVATE_CPS_API_KEY_UTILS_H_ */
