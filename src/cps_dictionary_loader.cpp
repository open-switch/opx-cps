/** OPENSOURCELICENSE */
/*
 * cps_dictionary_loader.cpp
 *
 *  Created on: Aug 5, 2015
 */

#include "cps_class_map.h"

#include "private/cps_dictionary.h"
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

    cps_api_attr_id_t key_id = CPS_API_ATTR_RESERVE_RANGE_END;
    cps_class_map_node_details key_d;
    key_d.desc = "CPS Internal Key info";
    key_d.embedded = true;
    key_d.name = "cps/key_data";
    key_d.attr_type = CPS_CLASS_ATTR_T_CONTAINER;
    key_d.data_type = CPS_CLASS_DATA_TYPE_T_EMBEDDED;
    cps_class_map_init(key_id,&key_id,1,&key_d);
}


