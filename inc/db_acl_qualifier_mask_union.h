
/* DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */
#ifndef __DB_ACL_QUALIFIER_MASK_UNION_H__
#define __DB_ACL_QUALIFIER_MASK_UNION_H__

typedef union {

  db_acl_valmask32_t valmask32;
  db_acl_valmask16_t valmask16;
  db_acl_valmask8_t valmask8;
  db_acl_macinfo_valmask_t macinfo_valmask;
  db_acl_ipaddr_valmask_t ipaddr_valmask;
  db_acl_ip6addr_valmask_t ip6addr_valmask;
  db_acl_ip_frag_mask_t ip_frag_mask;
  db_acl_srcmac_mask_t srcmac_mask;
  db_acl_dstmac_mask_t dstmac_mask;
  db_acl_outervlan_mask_t outervlan_mask;
  db_acl_srcip_mask_t srcip_mask;
  db_acl_dstip_mask_t dstip_mask;
  db_acl_srcip6_mask_t srcip6_mask;
  db_acl_dstip6_mask_t dstip6_mask;
  db_acl_portclass_mask_t portclass_mask;
  db_acl_ethertype_mask_t ethertype_mask;
  db_acl_intclassl2_mask_t intclassl2_mask;
  db_acl_ingstpstate_mask_t ingstpstate_mask;
  db_acl_fwdvlanvalid_mask_t fwdvlanvalid_mask;
  db_acl_l2stationmove_mask_t l2stationmove_mask;
  db_acl_ipflags_mask_t ipflags_mask;
  db_acl_ipproto_mask_t ipproto_mask;
  db_acl_ipttl_mask_t ipttl_mask;
  db_acl_dscp_mask_t dscp_mask;
  db_acl_srcclass_mask_t srcclass_mask;
  db_acl_srctrunk_mask_t srctrunk_mask;
  db_acl_dsttrunk_mask_t dsttrunk_mask;
  db_acl_tcpcontrol_mask_t tcpcontrol_mask;
  db_acl_packetres_mask_t packetres_mask;
  db_acl_vlaninfo_valmask_t vlaninfo_valmask;
  db_acl_color_mask_t color_mask;
  db_acl_l2_class_info_mask_t l2_class_info_mask;
  db_acl_l2format_mask_t l2format_mask;
  db_acl_inport_mask_t inport_mask;
  db_acl_outport_mask_t outport_mask;
  db_acl_dstport_mask_t dstport_mask;
  db_acl_l4srcport_mask_t l4srcport_mask;
  db_acl_l4dstport_mask_t l4dstport_mask;
  db_acl_ip_frag_mask_t ip_frag_mask;
  db_acl_ip_type_mask_t ipype_mask;
  db_acl_icmp_mask_t icmp_mask;
  db_acl_tunnel_type_mask_t tunnelype_mask;

} db_acl_qualifier_mask_t;

#endif
