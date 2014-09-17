/*
 * filename: db_acl_actions.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

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
    db_acl_PERMIT=1,
    db_acl_DENY,
    db_acl_REDIRECT,
    db_acl_PKT_PRIO,
    db_acl_COPY_TO_CPU,
    db_acl_COPY_TO_CPU_CANCEL,
    db_acl_SWITCH_TO_CPU_CANCEL,
    db_acl_COSQ_CPU_NEW,
    db_acl_DROP_PRECEDENCE,
    db_acl_METER_CONFIG,
    db_acl_RP_DROP_PRECEDENCE,
    db_acl_YP_DROP_PRECEDENCE,
    db_acl_SWITCH_CANCEL,
    db_acl_PBRREDIRECT,
    db_acl_COSQ_NEW,
    db_acl_RP_DROP,
    db_acl_REDIRECT_IPMC,
    db_acl_ECN_NEW,
    db_acl_CLASS_TAG,
    db_acl_REDIRECT_VLAN,
    db_acl_CLASS_DEST,
    db_acl_REDIRECT_TRUNK,
    db_acl_REDIRECT_PBMP,
    db_acl_SRCMAC_NEW,
    db_acl_DSTMAC_NEW,
    db_acl_OUTERVLAN_NEW,
    db_acl_OUTERVLANPCP_NEW,
    db_acl_DSCP_NEW,
    db_acl_EGRESS_PORTS_ADD,
    db_acl_CLASS_DEST_SET,
    db_acl_CLASS_SRC_SET,
    db_acl_L3_CHANGE_MAC_DA,
    db_acl_EGRESS_MASK,
    db_acl_REDIRECT_MCAST,
    db_acl_MIRROR,
    db_acl_MIRROR_INGRESS,
    db_acl_MIRROR_EGRESS,
    db_acl_MIRROR_OVERRIDE,
    db_acl_REDIRECT_L2MC
}db_acl_actioninfo;

typedef struct _action
{
    db_acl_actioninfo action;
    int param1;
    int param2;
}db_acl_action_t;



#endif /* DB_ACL_ACTIONS_H_ */
