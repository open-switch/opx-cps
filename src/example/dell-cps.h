
/*
* source yang file : dell-cps.yin
* (c) Copyright 2014 Dell Inc. All Rights Reserved.
*/

/* OPENSOURCELICENSE */
#ifndef DELL_CPS_H
#define DELL_CPS_H


/* descripton of object and how to use */
typedef enum {
 OBJECT_LIST_TYPE_OBJ_LIST_IX = 1/*!< uint32 */,
 OBJECT_LIST_TYPE_OBJ_ID = 2/*!< uint32 */,
 OBJECT_LIST_TYPE_NAME = 3/*!< string */,
 OBJECT_LIST_TYPE_UINT16_TYPE_FIELD = 4/*!< uint16 */,
 OBJECT_LIST_TYPE_GET_ATTRIBUTE_ONLY = 5/*!< uint32 */,
} OBJECT_LIST_TYPE_t;

/* This module is an example for people implementing CPS modules. */
typedef enum {
 DELL_CPS_OBJECT_LIST_TYPE = 1/*!< OBJECT_LIST_TYPE_t */,
} DELL_CPS_t;

typedef enum { 
  ENUMERATION_NAME_TYPE_ENUMERATION_ONE = 1,
  ENUMERATION_NAME_TYPE_ENUMERATION_TWO = 2,
} ENUMERATION_NAME_TYPE_t ; 

#endif
