/** OPENSOURCELICENSE */
/*
 * db_acl_qualifiers.h
 */

#ifndef DB_ACL_QUALIFIERS_H_
#define DB_ACL_QUALIFIERS_H_

#include "db_common_types.h"
#include "std_error_codes.h"
#include "db_common_list.h"

typedef enum
{
 ACL_SRC_MAC = 0,
 ACL_ETHER_TYPE,
 ACL_COLOR,
 ACL_INT_CLASS_L2,
 ACL_ING_PORT,
 ACL_EGR_PORT,
 ACL_IP_PROTO,
 ACL_IP_TTL,
 ACL_IP_FLAGS,
 ACL_IP_SRC,
 ACL_IP_DST,
 ACL_IP6_SRC,
 ACL_IP6_DST,
 ACL_DST_L2_PORT,
 ACL_DST_MAC,
 ACL_SRC_MAC,
 ACL_OUTER_VLAN,
 ACL_SRC_L4_PORT,
 ACL_DST_L4_PORT,
 ACL_DSCP,
 ACL_ING_STP_STATE,
 ACL_FWD_VLAN_VALID,
 ACL_IP_FRAG,
 ACL_SRC_CLASS,
 ACL_ICMP_TYPE,
 ACL_SRC_TRUNK,
 ACL_LPBK_TYPE,
 ACL_DST_CLASS,
 ACL_INT_CLASS_PORT,
 ACL_TUNNEL_TYPE,
 ACL_L2_STATION_MOVE,
 ACL_TCP_CONTROL,
 ACL_LAST_QUALIFIER
}db_acl_qualifierinfo;

typedef struct _action
{
    db_acl_qualifierinfo qualifier;
    int param1;
    int param2;
}db_acl_qualifier_t;

#endif /* DB_ACL_QUALIFIERS_H_ */
