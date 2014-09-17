/*
 * filename: db_interface_common.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/** OPENSOURCELICENSE */
/*
 * db_interface_common.cpp
 */
#include "db_interface.h"
#include <string.h>

void db_interface_ifname_copy(hal_ifname_t *out,const hal_ifname_t* const in) {
    (*out) [ sizeof (*out)-1 ] = 0;
    strncpy((*out),(*in),sizeof(*out));
}

