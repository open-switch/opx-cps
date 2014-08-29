PROGRAMS=
SUBDIRS=src
LIBRARIES=
ARCHIVES=
HEADERS=$(wildcard inc/*) inc/db_acl_qualifier_mask_union.h

include ${PROJROOT}/tools/workspace.mak

inc/db_acl_qualifier_mask_union.h : 
	bin/mk_acl_union2.sh $@ db_acl_qualifier_mask_union_h inc/db_acl_qualifier_masks.h
