PROGRAMS=
SUBDIRS=
LIBRARIES= db_common
ARCHIVES=
HEADERS=$(wildcard inc/*)

db_common_SRCS=db_common_list.cpp db_interface_common.cpp db_operation_common.cpp

include ${PROJROOT}/tools/workspace.mak

db_acl_qualifiers.h :
	../../hal-acl/bin/csv_to_enum.sh ../../hal-acl/bin/acl_field_mapping.csv $@ qualifiers

db_acl_actions.h :
	../../hal-acl/bin/csv_to_enum.sh ../../hal-acl/bin/acl_action_mapping.csv $@ actions
