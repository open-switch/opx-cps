/*
 * Copyright (c) 2016 Dell Inc.
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
 */

/*
 * cps_dictionary_loader.cpp
 *
 *  Created on: Aug 5, 2015
 */

#include "cps_class_map.h"
#include "cps_class_map_query.h"
#include "cps_api_metadata_import.h"

#include "cps_dictionary.h"
#include "cps_api_operation_stats.h"
#include "cps_class_map_query.h"

#include "std_envvar.h"
#include "std_directory.h"
#include "std_mutex_lock.h"
#include "std_shlib.h"
#include "std_utils.h"
#include "event_log.h"

#include "dell-cps.h"

#include <sys/stat.h>
#include <string>
#include <unordered_map>
#include <mutex>

static std_mutex_lock_create_static_init_rec(lock);
static std::unordered_map<std::string,struct stat> _loaded_libs;

static bool _cps_class_data(const char *name, std_dir_file_TYPE_t type,void *context) {
    if (name==nullptr) return true;
    if (type != std_dir_file_T_FILE) return true;

    std::string _dup_check = name;
    if ((context!=nullptr) && _dup_check.find((const char*)context)==std::string::npos) {
        return true;
    }

    struct stat _stats;
    if (stat(name,&_stats)!=0) {
        memset(&_stats,0,sizeof(_stats));
    }

    auto _loc = _dup_check.rfind('/');
    if (_loc!=std::string::npos) {
        _dup_check = _dup_check.substr(_loc+1);
    }

    _loc = _dup_check.find('.');

    if (_loc!=std::string::npos) {
        _dup_check = _dup_check.substr(0,_loc);
    }

    std_mutex_simple_lock_guard lg(&lock);

    //avoid duplicate loaded libs..
    if (_loaded_libs.find(_dup_check)!=_loaded_libs.end()) {
        return true;
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
             _loaded_libs[_dup_check] = _stats;

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

void _cps_api_class_map_init(void) {
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

    cps_api_yang_module_init();

    cps_api_metadata_import();
}

static std::mutex *_init_lock = new std::mutex;
void cps_api_class_map_init(void) {
    {
        std::lock_guard<std::mutex> l(*_init_lock);
        static bool loaded = false;
        if(loaded) return;
        loaded = true;
    }
    _cps_api_class_map_init();
}

