PROGRAMS=
SUBDIRS=src
LIBRARIES=
ARCHIVES=
HEADERS=$(wildcard inc/*) inc/db_acl_qualifier_mask_union.h inc/db_acl_qualifier.h inc/db_acl_action.h

include ${PROJROOT}/tools/workspace.mak

inc/db_acl_qualifier_mask_union.h : 
	bin/mk_acl_union2.sh $@ db_acl_qualifier_mask_union_h inc/db_acl_qualifier_masks.h

inc/db_acl_qualifier.h :
	../hal-acl/bin/csv_to_enum.sh ../hal-acl/bin/acl_field_mapping.csv $@ qualifier

inc/db_acl_action.h :
	../hal-acl/bin/csv_to_enum.sh ../hal-acl/bin/acl_action_mapping.csv $@ action
