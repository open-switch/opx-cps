/** OPENSOURCELICENSE */
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
        void (*class_data_init)(void);
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
        { CPS_API_ATTR_RESERVE_RANGE_END,{(cps_api_attr_id_t)CPS_API_ATTR_RESERVE_RANGE_END},
            { "cps/key_data", "CPS Internal Key info", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_EMBEDDED }
        },
        { cps_api_obj_cat_CPS_OBJ,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ},
            { "cps/object", "CPS Object Registration details", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_EMBEDDED }
        },
        { cps_api_obj_stat_SET_MIN_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_MIN_TIME},
            { "cps/object/set_min_time", "The minimum amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_MAX_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_MAX_TIME},
            { "cps/object/set_max_time", "The max amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_AVE_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_AVE_TIME},
            { "cps/object/set_ave_time", "The ave amount of time for a trans function", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_COUNT,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_COUNT},
            { "cps/object/set_requests", "The number of set/transaction functions", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_GET_MIN_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_MIN_TIME},
            { "cps/object/get_min_time", "The minimum amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_MAX_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_MAX_TIME},
            { "cps/object/get_max_time", "The max amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_AVE_TIME,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_AVE_TIME},
            { "cps/object/get_ave_time", "The ave amount of time for a get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_COUNT,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_COUNT},
            { "cps/object/get_requests", "The number of get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_NS_CONNECTS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_NS_CONNECTS},
            { "cps/object/nameservice_reconnects", "Number of times re-connected to the NS", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_NS_DISCONNECTS,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_NS_DISCONNECTS},
            { "cps/object/nameservice_lost_conn", "Number of times that the NS disconnected", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_SET_FAILED,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_FAILED},
            { "cps/object/set_failed", "Number of failed set requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_SET_INVALID,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_SET_INVALID},
            { "cps/object/set_invalid_req", "Number of invalid set requests (communication error)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

        { cps_api_obj_stat_GET_FAILED,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_FAILED},
            { "cps/object/get_failed", "Number of failed get requests", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },
        { cps_api_obj_stat_GET_INVALID,{(cps_api_attr_id_t)cps_api_obj_cat_CPS_OBJ,cps_api_obj_stat_GET_INVALID},
            { "cps/object/get_invalid_req", "Number of invalid get requests (communication error)", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }
        },

    };
    ix = 0;
    mx = sizeof(internal)/sizeof(*internal);
    for ( ; ix < mx ; ++ix ) {
        cps_class_map_init(internal[ix].id,&internal[ix].keys[0],internal[ix].keys.size(),&internal[ix].details);
    }

}


