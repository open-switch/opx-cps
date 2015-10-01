/** OPENSOURCELICENSE */
/*
 * cps_api_operation_stats.h
 *
 *  Created on: Sep 22, 2015
 */

#ifndef CPS_API_INC_CPS_API_OPERATION_STATS_H_
#define CPS_API_INC_CPS_API_OPERATION_STATS_H_

/** @defgroup CPSAPI The CPS API
@{
*/


#include "cps_api_object_category.h"

typedef enum {
    cps_api_obj_stat_e_OPERATIONS = ((uint64_t)cps_api_obj_cat_CPS_OBJ<<16)
}cps_api_obj_stat_elements_t;


typedef enum {
    cps_api_obj_stat_BEGIN= cps_api_obj_stat_e_OPERATIONS + 1,
    cps_api_obj_stat_SET_MIN_TIME=cps_api_obj_stat_BEGIN, //!<cps_api_obj_stat_SET_MIN_TIME the minimum amount of time for a set request (us)
    cps_api_obj_stat_SET_MAX_TIME, //!<cps_api_obj_stat_SET_MAX_TIME the max amount of time for a set request (us)
    cps_api_obj_stat_SET_AVE_TIME, //!<cps_api_obj_stat_SET_AVE_TIME the ave amount of time for a set request (us)
    cps_api_obj_stat_SET_COUNT,  //!<cps_api_obj_stat_SET_COUNT the number of set requests processed

    cps_api_obj_stat_GET_MIN_TIME, //!<cps_api_obj_stat_GET_MIN_TIME the minimum amount of time for a get request (us)
    cps_api_obj_stat_GET_MAX_TIME,//!<cps_api_obj_stat_GET_MAX_TIME the max amount of time for a get request (us)
    cps_api_obj_stat_GET_AVE_TIME, //!<cps_api_obj_stat_GET_AVE_TIME the ave amount of time for a get request (us)
    cps_api_obj_stat_GET_COUNT, //!<cps_api_obj_stat_GET_COUNT the number of get requests processed

    cps_api_obj_stat_NS_CONNECTS, //!<cps_api_obj_stat_NS_CONNECTS number of connections to the NS
    cps_api_obj_stat_NS_DISCONNECTS,//!<cps_api_obj_stat_NS_DISCONNECTS number of disconnects from the NS

    cps_api_obj_stat_SET_FAILED, //!<cps_api_obj_stat_SET_FAILED number of failed sets
    cps_api_obj_stat_SET_INVALID,//!<cps_api_obj_stat_SET_INVALID number of invalid sets

    cps_api_obj_stat_GET_FAILED,//!<cps_api_obj_stat_GET_FAILED number of failed gets
    cps_api_obj_stat_GET_INVALID,//!<cps_api_obj_stat_GET_INVALID number of invalid gets

    cps_api_obj_stat_KEY, //!<cps_api_obj_stat_KEY the key field indicating that a number of stats will be added
                            //!< related to this key
    cps_api_obj_stat_CLOSE_COUNT, //!<number of closed connections over the stats lifetime
    cps_api_obj_stat_CLOSE_CLEANUP_RUNS,//!<number of times that the cleanup has run on registrations
    cps_api_obj_stat_EVENT_SEND,//!<number of events sent
    cps_api_obj_stat_MAX
} cps_api_obj_stats_type_t;


/**
@}
*/
#endif /* CPS_API_INC_CPS_API_OPERATION_STATS_H_ */
