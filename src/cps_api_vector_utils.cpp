
#include "cps_api_vector_utils.h"

#include <stddef.h>
#include <string.h>

bool cps_utils::cps_api_vector_util_append(std::vector<char> &lst_,const void *data_, size_t len_) {
    size_t len = lst_.size();
    try {
        lst_.resize(len_ + len);
    } catch (...) {
        return false;
    }
    memcpy(&lst_[len],data_,len_);
    return true;
}
