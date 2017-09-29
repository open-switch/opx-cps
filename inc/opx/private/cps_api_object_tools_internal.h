/*
 * cps_api_object_tools_internal.h
 *
 *  Created on: May 19, 2017
 *      Author: cwichmann
 */

#ifndef CPS_API_OBJECT_TOOLS_INTERNAL_H_
#define CPS_API_OBJECT_TOOLS_INTERNAL_H_

#include "cps_api_object.h"

static inline bool cps_api_obj_attr_get_bool(cps_api_object_t obj, cps_api_attr_id_t id, bool default_value=false) {
   const bool *_p = (const bool*)cps_api_object_get_data(obj,id);
	return _p==nullptr ? default_value : *_p;
}


#endif /* CPS_API_OBJECT_TOOLS_INTERNAL_H_ */
