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

/**
 * @file cps_class_ut_data.cpp
 */

#include "cps_class_ut_data.h"
#include "cps_class_map.h"
#include "private/cps_string_utils.h"

#include "cps_class_map_query.h"
#include "cps_api_metadata_import.h"


#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>

static std::vector<_node_details> lst = {
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

std::vector<_node_details> &__get_class_map(){ return lst; }

cps_api_object_list_guard _100(cps_api_object_list_create());
cps_api_object_list_t Get100() {
    return _100.get();
}
cps_api_object_list_guard _1000(cps_api_object_list_create());
cps_api_object_list_t Get1000() {
    return _1000.get();
}
cps_api_object_list_guard _10000(cps_api_object_list_create());
cps_api_object_list_t Get10000() {
    return _10000.get();
}
cps_api_object_list_guard _100000(cps_api_object_list_create());
cps_api_object_list_t Get100000() {
    return _100000.get();
}

cps_api_object_list_guard _1000000(cps_api_object_list_create());
cps_api_object_list_t Get1000000() {
    return _1000000.get();
}

cps_api_object_list_guard _big(cps_api_object_list_create());
cps_api_object_list_t GeBIG() {
    return _big.get();
}

void __init_global_test_objects() {
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    size_t ix = 0;
    size_t mx = 100000;
    for ( ; ix < mx ; ++ix ) {

        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);

        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,ix);

        std::string _elem = cps_string::sprintf("%s-%d","Cliff",(int)ix);

        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
        cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_elem.c_str(),_elem.size());

        if (ix < 100) {
            cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
            cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,1);
            cps_api_object_t o = cps_api_object_list_create_obj_and_append(_100.get());
            cps_api_object_clone(o,og.get());
        }
        if (ix < 1000) {
            cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
            cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,2);
            cps_api_object_t o = cps_api_object_list_create_obj_and_append(_1000.get());
            cps_api_object_clone(o,og.get());
        }
        if (ix < 10000) {
            cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
            cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,3);
            cps_api_object_t o = cps_api_object_list_create_obj_and_append(_10000.get());
            cps_api_object_clone(o,og.get());
        }
        if (ix < 100000) {
            cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
            cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,4);
            cps_api_object_t o = cps_api_object_list_create_obj_and_append(_100000.get());
            cps_api_object_clone(o,og.get());
        }
    }
    char _tmpl[]="/tmp/UT_xxXXXXXX";
    char *_dir_name = mkdtemp(_tmpl);

    if(_dir_name==nullptr) assert(0);

    std::string _fn = _dir_name;
    _fn+=std::string("/blah")+CPS_DEF_CLASS_XML_SUFFIX;

    FILE *fp = fopen(_fn.c_str(),"w");
    assert(fp!=nullptr);

    std::string _meta_data_file = "<cps-class-map >\n"
"\n"
"<metadata_entry key=\"[cps]\" id=\"2\" name=\"cps\" desc=\"The CPS embedded space.\" embedded=\"true\" node-type=\"subsystem\" data-type=\"bin\" owner-type=\"db\" />\n"
"\n"
"<!-- Key data container-->\n"
"<metadata_entry key=\"[cps].[cps/key_data]\" id=\"0xffffffffffffffff\" name=\"cps/key_data\" desc=\"The CPS key data conainer.\" embedded=\"true\" node-type=\"container\" data-type=\"bin\" />\n"
"\n"
"<!-- CPS Object attributes -->\n"
"<metadata_entry key=\"[cps].[cps/object]\" id=\"0x8\" name=\"cps/object\" desc=\"This object contains attributes related to CPS operations on an object\" embedded=\"true\" node-type=\"container\" data-type=\"bin\" />\n"
"\n"
"<!-- CPS Service Statistics  -->\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat]\" id=\"0x80000\" name=\"cps/object/stat\" desc=\"Contains the CPS internal statisics for this object.\" embedded=\"true\" node-type=\"container\" data-type=\"bin\" />\n"
"\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_min_time]\" id=\"0x80001\" name=\"cps/object/operations/set_min_time\" desc=\"the fastest response to a set request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_max_time]\" id=\"0x80002\" name=\"cps/object/operations/set_max_time\" desc=\"the longest time it took to process a set request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_ave_time]\" id=\"0x80003\" name=\"cps/object/operations/set_ave_time\" desc=\"the average time it takes to process a set request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_requests]\" id=\"0x80004\" name=\"cps/object/operations/set_requests\" desc=\"the number of set requests.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_min_time]\" id=\"0x80005\" name=\"cps/object/operations/get_min_time\" desc=\"the fastest response to a get request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_max_time]\" id=\"0x80006\" name=\"cps/object/operations/get_max_time\" desc=\"the longest time it took to process a get request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_ave_time]\" id=\"0x80007\" name=\"cps/object/operations/get_ave_time\" desc=\"the average time it takes to process a get request.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_requests]\" id=\"0x80008\" name=\"cps/object/operations/get_requests\" desc=\"the number of get requests.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/nameservice_reconnects]\" id=\"0x80009\" name=\"cps/object/operations/nameservice_reconnects\" desc=\"the number of reconnections with the name service.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/nameservice_lost_conn]\" id=\"0x8000a\" name=\"cps/object/operations/nameservice_lost_conn\" desc=\"Number of times that the NS disconnected.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_failed]\" id=\"0x8000b\" name=\"cps/object/operations/set_failed\" desc=\"Number of failed set requests.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/set_invalid_req]\" id=\"0x8000c\" name=\"cps/object/operations/set_invalid_req\" desc=\"Number of invalid set requests (communication error).\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_failed]\" id=\"0x8000d\" name=\"cps/object/operations/get_failed\" desc=\"Number of failed get requests.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/get_invalid_req]\" id=\"0x8000e\" name=\"cps/object/operations/get_invalid_req\" desc=\"Number of invalid set requests due to protocol errors.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/key_field]\" id=\"0x8000f\" name=\"cps/object/operations/key_field\" desc=\"The keys registered with this service\" embedded=\"false\" node-type=\"leaf-list\" data-type=\"key\" />\n"
"\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/close_count]\" id=\"0x80010\" name=\"cps/object/operations/close_count\" desc=\"Number of close operations done with the nameservice.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/cleanup_runs]\" id=\"0x80011\" name=\"cps/object/operations/cleanup_runs\" desc=\"Number of clean ups on cache done due to close connections.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/events_sent]\" id=\"0x80012\" name=\"cps/object/operations/events_sent\" desc=\"Number of events sent by the name service for registration changes.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"<metadata_entry key=\"[cps].[cps/object].[cps/object/stat].[cps/object/operations/process_id]\" id=\"0x80013\" name=\"cps/object/operations/process_id\" desc=\"The process ID of the application registering for object handling.\" embedded=\"false\" node-type=\"leaf\" data-type=\"uint64_t\" />\n"
"\n"
"<!-- CPS Node Ownership  -->\n"
"<class_ownership id=\"cps/node-group\" qualifiers=\"target,observed\" owner-type=\"db\" />\n"
"\n"
"\n"
"<!-- CPS Enum Mapping  -->\n"
"<enum_entry name=\"base-acl/counter-type\" >\n"
"  <enum name=\"PACKET\" value=\"1\" />\n"
"  <enum name=\"BYTE\" value=\"2\" />\n"
"</enum_entry>\n"
"\n"
"<enum_association name=\"base-acl/counter/types\" enum=\"base-acl/counter-type\" />\n"
"\n"
"</cps-class-map>\n"
"\n"
;
    assert(fwrite(_meta_data_file.c_str(),_meta_data_file.size(),1,fp)>0);
    fclose(fp);

    setenv(CPS_API_METADATA_ENV,_dir_name,true);

    cps_api_metadata_import();
    std::string _cmd = "rm -Rf ";
    _cmd += _dir_name;
    assert(system(_cmd.c_str())==0);

}

void __init_class_map() {
    std::vector<cps_api_attr_id_t> ids ;

    for ( auto &it : lst) {
        cps_class_map_init(it.id,&(it._ids[0]),it._ids.size(),&it.details);
    }
    __init_global_test_objects();
}
