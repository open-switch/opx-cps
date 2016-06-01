/*
 * cps_api_vector_utils.h
 *
 *  Created on: May 23, 2016
 *      Author: cwichmann
 */

#ifndef CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_
#define CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_

#include <stddef.h>

#include <vector>

namespace cps_utils {

bool cps_api_vector_util_append(std::vector<char> &lst,const void *data, size_t len);

inline bool cps_api_vector_util_set(std::vector<char> &lst,const void *data, size_t len) {
	lst.clear();
	return cps_api_vector_util_append(lst,data,len);
}


}



#endif /* CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_ */
