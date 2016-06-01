/*
dell-cps
*/
#include "dell-cps.h"
#include "cps_class_map.h"

#include <vector>

static struct {
  std::vector<cps_api_attr_id_t> _ids;
  cps_api_attr_id_t id;
  cps_class_map_node_details details;
} lst[] = {
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_ALIAS }, CPS_NODE_DETAILS_ALIAS, { "cps/node-details/alias", "", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP }, CPS_OBJECT_GROUP, { "cps/object-group", "", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP }, CPS_OBJECT_GROUP_GROUP, { "cps/object-group/group", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_TYPE }, CPS_NODE_GROUP_TYPE, { "cps/node-group/type", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_NAME }, CPS_NODE_GROUP_NAME, { "cps/node-group/name", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_TRANSACTION }, CPS_OBJECT_GROUP_TRANSACTION, { "cps/object-group/transaction", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT64 }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME }, CPS_NODE_DETAILS, { "cps/node-details", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_DETAILS,CPS_NODE_DETAILS_NAME,CPS_NODE_DETAILS_NAME }, CPS_NODE_DETAILS_NAME, { "cps/node-details/name", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME,CPS_NODE_GROUP_IP }, CPS_NODE_GROUP_IP, { "cps/node-group/ip", "", false, CPS_CLASS_ATTR_T_LEAF_LIST, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS }, cps_api_obj_CAT_CPS, { "cps", "", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_CPS,CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NODE }, CPS_OBJECT_GROUP_NODE, { "cps/object-group/node", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_CPS,CPS_NODE_GROUP,CPS_NODE_GROUP_NAME }, CPS_NODE_GROUP, { "cps/node-group", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
};


static const size_t lst_len = sizeof(lst)/sizeof(*lst);
extern "C"{ 
  t_std_error cps_api_yang_module_init(void) {
    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) { 
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details); 
    }
    return STD_ERR_OK;
  }
}
