/** OPENSOURCELICENSE */
/*
 * db_acl_actions.h
 */

#ifndef DB_ACL_ACTIONS_H_
#define DB_ACL_ACTIONS_H_

#include "db_common_types.h"
#include "std_error_codes.h"
#include "db_common_list.h"


typedef enum
{
    PERMIT=1,
    DENY,
    REDIRECT,
    PKT_PRIO,
    COPY_TO_CPU,
    COPY_TO_CPU_CANCEL,
    SWITCH_TO_CPU_CANCEL,
    COSQ_CPU_NEW,
    DROP_PRECEDENCE,
    METER_CONFIG,
    RP_DROP_PRECEDENCE,
    YP_DROP_PRECEDENCE,
    SWITCH_CANCEL,
    PBRREDIRECT,
    COSQ_NEW, 
    RP_DROP,
    REDIRECT_IPMC,
    ECN_NEW,
    CLASS_TAG,
    REDIRECT_VLAN,
    CLASS_DEST,
    REDIRECT_TRUNK,
    REDIRECT_PBMP,
    SRCMAC_NEW,
    DSTMAC_NEW,
    OUTERVLAN_NEW,
    OUTERVLANPCP_NEW,
    DSCP_NEW,
    EGRESS_PORTS_ADD,
    CLASS_DEST_SET,
    CLASS_SRC_SET,
    L3_CHANGE_MAC_DA,
    EGRESS_MASK,
    REDIRECT_MCAST,
    MIRROR,
    MIRROR_INGRESS,
    MIRROR_EGRESS,
    MIRROR_OVERRIDE,
    REDIRECT_L2MC    
}db_acl_actioninfo;

typedef struct _action
{
    db_acl_actioninfo action;
    int param1;
    int param2;
}db_acl_action_t;



#endif /* DB_ACL_ACTIONS_H_ */
