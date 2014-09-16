PROGRAMS=
SUBDIRS=src
LIBRARIES=
ARCHIVES=
HEADERS=$(wildcard inc/*) inc/db_acl_qualifier_mask_union.h inc/db_acl_qualifier.h inc/db_acl_action.h

include ${PROJROOT}/tools/workspace.mak
