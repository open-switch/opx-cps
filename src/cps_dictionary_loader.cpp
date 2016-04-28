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

/*
 * cps_dictionary_loader.cpp
 *
 *  Created on: Aug 5, 2015
 */

#include "cps_class_map.h"

#include "private/cps_dictionary.h"
#include "cps_api_operation_stats.h"

#include "std_envvar.h"
#include "std_directory.h"
#include "std_mutex_lock.h"
#include "std_shlib.h"
#include "std_utils.h"
#include "event_log.h"

#include <sys/stat.h>
#include <string>
#include <map>

static std_mutex_lock_create_static_init_rec(lock);
static std::map<std::string,struct stat> _loaded_libs;

static bool _cps_class_data(const char *name, std_dir_file_TYPE_t type,void *context) {
    if (type != std_dir_file_T_FILE) return true;


     struct stat _stats;
     if (stat(name,&_stats)!=0) {
         memset(&_stats,0,sizeof(_stats));
     }

     std_mutex_simple_lock_guard lg(&lock);

    //avoid duplicate loaded libs..
    if (_loaded_libs.find(name)!=_loaded_libs.end()) {
        if (_loaded_libs[name].st_ino == _stats.st_ino) return true;
    }
    if (strstr(name,(const char*)context)!=NULL) {
        void (*class_data_init)(void)=nullptr;
         static std_shlib_func_map_t func_map[] = {
             { "module_init", (void **)&class_data_init }
         };
         static const size_t func_map_size = sizeof(func_map)/sizeof(*func_map);
         std_shlib_hndl lib_hndl = STD_SHLIB_INVALID_HNDL;

         if (STD_ERR_OK != std_shlib_load(name, &lib_hndl, func_map, func_map_size)) {
             EV_LOG(ERR,DSAPI,0,"cps_class_data","Can not load function map");
         } else {
             _loaded_libs[name] = _stats;

             class_data_init();
             //Since we don't need to use any functions in the library after initialized
             //then we can unload the library
             std_shlib_unload(lib_hndl);
         }

    }
    return true;
}

bool cps_class_objs_load(const char *path, const char * prefix) {
    t_std_error rc = std_dir_iterate(path,_cps_class_data,(void*)prefix,false);
    return rc==STD_ERR_OK;
}

void cps_api_class_map_init(void) {
    const char * path = std_getenv("LD_LIBRARY_PATH");
    if (path==NULL) {
        path = CPS_DEF_SEARCH_PATH;
    }
    std_parsed_string_t handle = NULL;
    if (!std_parse_string(&handle,path,":")) {
        return;
    }
    size_t ix = 0;
    size_t mx =  std_parse_string_num_tokens(handle);
    for ( ; ix < mx ; ++ix ) {
       const char * p = std_parse_string_at(handle,ix);
       cps_class_objs_load(p,CPS_DEF_CLASS_FILE_NAME);
    }
    std_parse_string_free(handle);
    struct {
        cps_api_attr_id_t id;
        std::vector<cps_api_attr_id_t> keys;
        cps_class_map_node_details details;
    } internal[] = {
            // \todo move to a yang model or xml file that can be read at startup
        { CPS_API_ATTR_RESERVE_RANGE_END,{(cps_api_attr_id_t)CPS_API_ATTR_RESERVE_RANGE_END},
            { "cps/key_data", "CPS Internal Key info", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_EMBEDDED }
        },
        { cps_api_obj_cat_CPS_OBJ,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ},
            { "cps/object", "CPS Object Registration details", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_EMBEDDED }
        },
        { cps_api_obj_stat_e_OPERATIONS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS},
            { "cps/object/operations", "Contain the operational stats", false, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_EMBEDDED }
        },
        { cps_api_obj_stat_SET_MIN_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_MIN_TIME},
            { "cps/object/operations/set_min_time", "The minimum amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_MAX_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_MAX_TIME},
            { "cps/object/operations/set_max_time", "The max amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_AVE_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_AVE_TIME},
            { "cps/object/operations/set_ave_time", "The ave amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_COUNT,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_COUNT},
            { "cps/object/operations/set_requests", "The number of set/transaction functions", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_GET_MIN_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_MIN_TIME},
            { "cps/object/operations/get_min_time", "The minimum amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_MAX_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_MAX_TIME},
            { "cps/object/operations/get_max_time", "The max amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_AVE_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_AVE_TIME},
            { "cps/object/operations/get_ave_time", "The ave amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_COUNT,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_COUNT},
            { "cps/object/operations/get_requests", "The number of get requests", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_NS_CONNECTS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_NS_CONNECTS},
            { "cps/object/operations/nameservice_reconnects", "Number of times re-connected to the NS", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_NS_DISCONNECTS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_NS_DISCONNECTS},
            { "cps/object/operations/nameservice_lost_conn", "Number of times that the NS disconnected", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_SET_FAILED,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_FAILED},
            { "cps/object/operations/set_failed", "Number of failed set requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_INVALID,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_SET_INVALID},
            { "cps/object/operations/set_invalid_req", "Number of invalid set requests (communication error)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_GET_FAILED,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_FAILED},
            { "cps/object/operations/get_failed", "Number of failed get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_INVALID,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_GET_INVALID},
            { "cps/object/operations/get_invalid_req", "Number of invalid get requests (communication error)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_KEY,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_KEY},
            { "cps/object/operations/key_field", "The key related to the counts that will be following", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_KEY }
        },

        { cps_api_obj_stat_CLOSE_COUNT,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_CLOSE_COUNT},
            { "cps/object/operations/close_count", "Number of close operations done with the nameservice", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_CLOSE_CLEANUP_RUNS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_CLOSE_CLEANUP_RUNS},
            { "cps/object/operations/cleanup_runs", "Number of clean ups on cache done due to close connections", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_EVENT_SEND,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_EVENT_SEND},
            { "cps/object/operations/events_sent", "Number of events sent by the name service for registration changes", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_PROCESS_ID,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_e_OPERATIONS,cps_api_obj_stat_PROCESS_ID},
            { "cps/object/operations/process_id", "The process ID of the application registering for object handling", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

    };
    ix = 0;
    mx = sizeof(internal)/sizeof(*internal);
    for ( ; ix < mx ; ++ix ) {
        cps_class_map_init(internal[ix].id,&internal[ix].keys[0],internal[ix].keys.size(),&internal[ix].details);
    }

}


