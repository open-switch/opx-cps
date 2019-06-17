/*
 * Copyright (c) 2019 Dell Inc.
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
 * cps_api_event_loss_unittest.cpp
 */


#include "cps_api_events.h"
#include "cps_api_core_utils.h"

//IP Meta data
#include "cps_class_ut_data.h"

#include "std_time_tools.h"
#include "std_assert.h"

#include <pthread.h>
#include <thread>

#include <functional>
#include <mutex>
#include <stdint.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

static std::vector<_node_details> __lst = {
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP }, BASE_IP_IPV6_ADDRESS, { "base-ip/ipv6/address", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP,BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH }, BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH, { "base-ip/ipv4/address/prefix-length", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT8 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_FORWARDING }, BASE_IP_IPV6_FORWARDING, { "base-ip/ipv6/forwarding", "This will be set true if the system has IP forwarding enabled.This will only set the kernel IP forwarding flags.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP }, cps_api_obj_CAT_BASE_IP, { "base-ip", "This model will support the configruation of IP address.***Depreciated****", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX }, BASE_IP_IPV6, { "base-ip/ipv6", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX }, BASE_IP_IPV4, { "base-ip/ipv4", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_NAME }, BASE_IP_IPV6_NAME, { "base-ip/ipv6/name", "The interfaces's name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_NAME }, BASE_IP_IPV4_NAME, { "base-ip/ipv4/name", "The interfaces's name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_VRF_ID }, BASE_IP_IPV4_VRF_ID, { "base-ip/ipv4/vrf-id", "A numerical value of the vrf that contains the interface.  Use 0 for the default.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_FORWARDING }, BASE_IP_IPV4_FORWARDING, { "base-ip/ipv4/forwarding", "This will be set true if the system has IP forwarding enabled.This will only set the kernel IP forwarding flags.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_VRF_ID }, BASE_IP_IPV6_VRF_ID, { "base-ip/ipv6/vrf-id", "A numerical value of the vrf that contains the interface.  Use 0 for the default.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_IFINDEX }, BASE_IP_IPV6_IFINDEX, { "base-ip/ipv6/ifindex", "The interface index", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP }, BASE_IP_IPV4_ADDRESS, { "base-ip/ipv4/address", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP,BASE_IP_IPV4_ADDRESS_IP }, BASE_IP_IPV4_ADDRESS_IP, { "base-ip/ipv4/address/ip", "IP address", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP,BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH }, BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH, { "base-ip/ipv6/address/prefix-length", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP,BASE_IP_IPV6_ADDRESS_IP }, BASE_IP_IPV6_ADDRESS_IP, { "base-ip/ipv6/address/ip", "IP address", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ENABLED }, BASE_IP_IPV4_ENABLED, { "base-ip/ipv4/enabled", "This is true if IPv4 is enabled.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ENABLED }, BASE_IP_IPV6_ENABLED, { "base-ip/ipv6/enabled", "This is true if IPv4 is enabled.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS }, BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS, { "base-ip/ipv6/dup-addr-detect-transmits", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_IFINDEX }, BASE_IP_IPV4_IFINDEX, { "base-ip/ipv4/ifindex", "The interface index", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
};

static void _init(void) {
    for ( auto &it : __lst) {
        cps_class_map_init(it.id,&(it._ids[0]),it._ids.size(),&it.details);
    }
}

static void _cps_api_publish_event(cps_api_object_t obj,
    std::function<void(cps_api_object_t)> change_cb, size_t count) {

    cps_api_event_service_handle_t handle;
    STD_ASSERT(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);
    for ( size_t _ix = 0; _ix < count ; ++_ix ) {
        cps_api_event_publish(handle,obj);
        change_cb(obj);
    }
    cps_api_event_client_disconnect(handle);
}

static cps_api_object_t _create_object() {
     cps_api_object_t obj = cps_api_object_create();
    STD_ASSERT(obj!=nullptr);
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,
        cps_api_qualifier_TARGET);
    cps_api_object_attr_add( obj,BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
    cps_api_object_attr_add( obj,BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
    return obj;
}

static uint32_t _inc_value=0;

//EVENT_TIME
//EVENT_PER_ITER
//EVENT_ITER_TIME

TEST(cps_api_event_loss,test_100_seconds) {

    ASSERT_TRUE(cps_api_event_service_init()==cps_api_ret_code_OK);

    size_t _vrf_id = time(nullptr);

    struct _env_vars {
        uint32_t     _length = 5;
        uint32_t     _burst = 15000;
        uint32_t     _burst_delay = 2000;
        uint32_t    _continue = 0;
    };

    struct _env_vars_defs {
        std::string _tag;
        uint32_t *_field_ptr;
    };

    _env_vars _vars;

    const std::vector<_env_vars_defs> _defns = {
        { "EVENT_LENGTH",&_vars._length },
        { "EVENT_BURST",&_vars._burst },
        { "EVENT_BURST_DELAY",&_vars._burst_delay }, //milli
        { "EVENT_IGNORE_ERROR",&_vars._continue }, //milli
    };

    uint32_t cnt = 0;
    size_t _lost_messages = 0;

    size_t _ix = 0;
    size_t _mx = _defns.size();
    for ( ; _ix < _mx ; ++_ix ) {
        const char *_str_val = getenv(_defns[_ix]._tag.c_str());
        if (_str_val!=nullptr) {
            *(_defns[_ix]._field_ptr) = atoi(_str_val);
        }
    }
    printf(    "Using length %u (s)\n"
             "Burst of %u\n"
            "Burst delay of %d (ms)\n",
            _vars._length, _vars._burst,_vars._burst_delay);

    std::mutex m1;
    m1.lock();
    _vars._length*=1000*1000;
    std::thread th([&]() {
        size_t _uptime = std_get_uptime(nullptr);
        cps_api_object_guard og(_create_object());
        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,_vrf_id);

        m1.lock();
        while (!std_time_is_expired(_uptime,_vars._length)) {
            _cps_api_publish_event(og.get(),[&](cps_api_object_t o){
                cps_api_object_attr_delete(o,_vrf_id);
                cps_api_object_attr_add_u64(o,_vrf_id,++_inc_value);
                if ((_inc_value%10000)==0) {
                    printf("Sent %d events - Received events %d - lost %d\n",
                        _inc_value,cnt,(int)_lost_messages);
                }
            },_vars._burst);
            std_usleep(_vars._burst_delay*1000);//in milli
        }
        printf("Finished sending\n");
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET));
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,_vrf_id);

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;

    cps_api_object_guard og(cps_api_object_create());

    cps_api_event_stats();

    m1.unlock();

    size_t _uptime = std_get_uptime(nullptr);

    while ( !std_time_is_expired(_uptime,(_vars._length*4)) ) {
        if (cps_api_timedwait_for_event(handle,og.get(),
            2000)!=cps_api_ret_code_OK) {
            bool _expired = std_time_is_expired(_uptime,_vars._length);
            printf("Invalid return from timed wait... (%d) \n",(int)_expired);
            if (_expired) {
                if ((_inc_value)==(cnt+1)) {
                    printf("Have all\n");
                    break;
                } else {
                    printf("%u and %u\n",_inc_value,cnt);
                }
            }
            continue;
        }
        uint32_t *_ptr = (uint32_t*) cps_api_object_get_data(og.get(),_vrf_id);
        if (_ptr!=nullptr) {
            ++cnt;
            if (*_ptr!=cnt) {
                size_t _now = std_get_uptime(nullptr);
                printf("Uptime.. %lu\n",_now);
                printf("Out of sequence old %u -> new %u\n",cnt,*_ptr);
                if (*_ptr > cnt) {
                    //lost
                    _lost_messages += *_ptr - cnt;
                }
                cnt = *_ptr;
                if (_vars._continue==0) exit(1);
            }
            if ((cnt%40000)==0) {
                printf("Received %d events lost %d\n",cnt,(int)_lost_messages);
            }
        } else {
            cps_api_object_print(og.get());
        }
    }
    sleep(1);
    printf("Received %u events\n",(uint32_t)cnt);
    cps_api_event_stats();

    th.join();
    cps_api_event_client_disconnect(handle);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  _init();
  return RUN_ALL_TESTS();
}
