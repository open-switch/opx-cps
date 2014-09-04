PROGRAMS=
SUBDIRS=src
LIBRARIES=
ARCHIVES=
HEADERS=$(wildcard inc/*) inc/db_acl_qualifier_mask_union.h inc/db_acl_qualifiers.h inc/db_acl_actions.h

include ${PROJROOT}/tools/workspace.mak

inc/db_acl_qualifier_mask_union.h : 
	bin/mk_acl_union2.sh $@ db_acl_qualifier_mask_union_h inc/db_acl_qualifier_masks.h

inc/db_acl_qualifiers.h :
	../hal-acl/bin/csv_to_enum.sh ../hal-acl/bin/acl_field_mapping.csv $@ qualifiers

inc/db_acl_actions.h :
	../hal-acl/bin/csv_to_enum.sh ../hal-acl/bin/acl_action_mapping.csv $@ actions
