#!/bin/bash

filen=$1
target_file=$2
union_name=db_acl_qualifier_mask_t

union_fields1=$(cat ../inc/db_acl_qualifier_masks.h  | grep \\\} | tr '}' ' '  | tr ';' ' '  | cut -f2 -d ' ')

union_fields2=$(cat ../inc/db_acl_qualifier_masks.h | grep \\mask_t\; | tr '}' ' ' | sed 's/typedef//' | sed -n -e 's/^.*valmask_t//p' | tr ';' ' ' )

union_fields3=$(cat ../inc/db_acl_qualifier_masks.h | grep \\mask_t\; | tr '}' ' ' | sed 's/typedef//' | sed -n -e 's/^.*valmask[0-9]_t//p' | tr ';' ' ' )

union_fields4=$(cat ../inc/db_acl_qualifier_masks.h | grep \\mask_t\; | tr '}' ' ' | sed 's/typedef//' | sed -n -e 's/^.*valmask[0-9][0-9]_t//p' | tr ';' ' ' )

real_union_fields=$union_fields1$union_fields2$union_fields3$union_fields4

echo "
#ifndef __${target_file}__
#define __${target_file}__

typedef union {
" > $filen

for i in $real_union_fields ; do
    fieldn=$(echo $i | sed -e 's/db_acl_//g' | sed -e 's/_t//g')
    echo "  $i ${fieldn};" >> $filen
done

echo "
} $union_name;

#endif" >> $filen
