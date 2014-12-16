/* OPENSOURCELICENSE */
/*
 * cps_api_qos.h
 */

#ifndef cps_api_qos_H_
#define cps_api_qos_H_


typedef enum hal_event_qos_types {
    cps_api_qos_obj_QDISC=1,//!< cps_api_qos_obj_QDISC
    cps_api_qos_obj_QOS_CLASS, //!< cps_api_qos_obj_QOS_CLASS
    cps_api_qos_obj_QOS_FILTER,//!< cps_api_qos_obj_QOS_FILTER
}cps_api_qos_sub_category_t;

//cps_api_qos_obj_QDISC
typedef enum {
    cps_api_if_QDISC_A_OPERATION=0, //db_qdisc_msg_type_t
    cps_api_if_QDISC_A_IFINDEX=1, //hal_ifindex_t
    cps_api_if_QDISC_A_TYPE=2, //db_qos_tca_type_t
    cps_api_if_QDISC_A_QDISC=3, //uint32_t
    cps_api_if_QDISC_A_PARENT=4, //uint32_t
}cps_api_if_QDISC_ATTR;

#if 0
struct db_qos_qdisc_entry_s {
    db_qdisc_msg_type_t msg_type;
    hal_ifindex_t  ifindex;
    db_qos_tca_type_t      qdisc_type;
    uint16_t        qdisc;
    uint32_t        parent;
};
#endif

//cps_api_qos_obj_QOS_CLASS
typedef enum {
    cps_api_qos_CLASS_A_OPERATION=0, //db_qdisc_msg_type_t
    cps_api_qos_CLASS_A_IFINDEX=1, //hal_ifindex_t
    cps_api_qos_CLASS_A_CLASS=2, //db_qos_tca_type_t
    cps_api_qos_CLASS_A_QDISC=3, //uint32_t
    cps_api_qos_CLASS_A_RATE_MIN_BW=4, //uint32_t
    cps_api_qos_CLASS_A_RATE_MAX_BW=5, //uint32_t
    cps_api_qos_CLASS_A_RATE_BURST=6, //uint32_t
    cps_api_qos_CLASS_A_RATE_CBURST=7,//uint32_t
}cps_api_qos_CLASS_ATTR;

#if 0
typedef struct db_class_rate_info_s {
    // Rate info of the node
    uint32_t minBw;
    uint32_t maxBw;
    uint32_t burst;
    uint32_t cburst;
} db_qos_class_rate_info_t;

struct db_qos_class_entry_s {
    db_qos_class_msg_type_t msg_type;
    hal_ifindex_t   ifindex;
    uint32_t        qos_class;
    uint16_t        qdisc;
    db_qos_class_rate_info_t rinfo;
};
#endif


#endif /* cps_api_qos_H_ */
