
/*
* source file : cps
* (c) Copyright 2015 Dell Inc. All Rights Reserved.
*/

/* OPENSOURCELICENSE */
#ifndef CPS_H
#define CPS_H

#include "cps_api_operation.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_CPS (29)

/*
Comments:Uniquely identifies any object in this file
*/
typedef uint32_t CPS_SOME_SPECIAL_ID_t;


/*Enumeration cps:enumeration-name-type */
typedef enum {
  CPS_ENUMERATION_NAME_TYPE_ENUMERATION_ONE = 1,
  CPS_ENUMERATION_NAME_TYPE_ENUMERATION_TWO = 3, /*if you know the value you need for the enumeration specify it but must be done for all enums in the enumeration*/
} CPS_ENUMERATION_NAME_TYPE_t;

/*Object cps/object-list-type */
typedef enum {
/*type=uint32*/
/*some description - eg rolls at x*/
  CPS_OBJECT_LIST_TYPE_OBJ_LIST_IX = 1900545,

/*type=uint32*/
/*part of the object's key*/
  CPS_OBJECT_LIST_TYPE_OBJ_ID = 1900546,

/*type=string*/
/*string name field - again some description*/
  CPS_OBJECT_LIST_TYPE_NAME = 1900547,

/*type=uint16*/
/*an example of another type*/
  CPS_OBJECT_LIST_TYPE_UINT16_TYPE_FIELD = 1900548,

/*type=uint32*/
/*this attribute is available for get - there is no configuration that can be done on it*/
  CPS_OBJECT_LIST_TYPE_GET_ATTRIBUTE_ONLY = 1900549,

/*type=binary*/
  CPS_OBJECT_LIST_TYPE_DO_IT = 1900550,

} CPS_OBJECT_LIST_TYPE_t;

/*Object cps/object-list-type/do-it/output */
typedef enum {
/*type=string*/
  CPS_OBJECT_LIST_TYPE_DO_IT_OUTPUT_OUTPUT = 1900551,

} CPS_OBJECT_LIST_TYPE_DO_IT_OUTPUT_t;

/*Object cps/object-list-type/do-it */
typedef enum {
/*type=binary*/
  CPS_OBJECT_LIST_TYPE_DO_IT_INPUT = 1900552,

/*type=binary*/
  CPS_OBJECT_LIST_TYPE_DO_IT_OUTPUT = 1900553,

} CPS_OBJECT_LIST_TYPE_DO_IT_t;

/*Object cps/object-list-type/do-it/input */
typedef enum {
/*type=string*/
  CPS_OBJECT_LIST_TYPE_DO_IT_INPUT_STATE = 1900554,

} CPS_OBJECT_LIST_TYPE_DO_IT_INPUT_t;

/* Object subcategories */
typedef enum{
/*descripton of object and how to use*/
  CPS_OBJECT_LIST_TYPE_OBJ = 1900555,

} CPS_OBJECTS_t;


#endif
