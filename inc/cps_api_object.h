/*
 * filename: cps_api_object.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/* OPENSOURCELICENSE */
/*
 * cps_api_object.h
 */

#ifndef CPS_API_COMMON_LIST_H_
#define CPS_API_COMMON_LIST_H_

#include "cps_api_key.h"

#include <stddef.h>
#include <stdbool.h>
#include "std_tlv.h"

#include "cps_api_object_attr.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CPSAPI The CPS API
 *
      This file consists of the utilities to create, and manage an object or list of
    objects.

    An object list contains a 0 or more objects

    An object contains a key and attributes.
       Each attribute contains an attribute id and a value.  The data can be either u16, u32, u64 or binary.

    An object currently can only be part of one object list. There is no reference count within the object.
    The object itself is not locking, so multi-threading issues must be handled by the
    calling applications.

@{
*/

/**
 * These are internally reserved attribute IDs for internal CPS usage.
 */
#define CPS_API_ATTR_RESERVE_RANGE_END ((cps_api_attr_id_t)-1)
#define CPS_API_ATTR_RESERVE_RANGE_START ((cps_api_attr_id_t)-10)

/*
 * These are some reserved IDs for the CPS
 * */
#define CPS_API_OBJ_KEY_ATTRS (CPS_API_ATTR_RESERVE_RANGE_END)

typedef enum cps_api_object_ATTR_TYPE_t {
    cps_api_object_ATTR_T_U16,
    cps_api_object_ATTR_T_U32,
    cps_api_object_ATTR_T_U64,
    cps_api_object_ATTR_T_BIN,
} cps_api_object_ATTR_TYPE_t;

/**
 * Return the cps_api_object_ATTR_TYPE for the given int size
 * @param len the length if the integer quantity (2,4,8)
 * @return the corresponding attribute type or BIN if not a valid size
 */
cps_api_object_ATTR_TYPE_t cps_api_object_int_type_for_len(size_t len);

/**
 * Each CPS Object has a object key along with a number of attributes.
 */
typedef void * cps_api_object_t;

#define CPS_API_OBJECT_NULL ((void*)0)

/*
 * The type of a list of objects
 */
typedef void * cps_api_object_list_t;

#define CPS_API_OBJ_OVERHEAD (100)
#define CPS_API_MIN_OBJ_LEN (sizeof(cps_api_key_t) + CPS_API_OBJ_OVERHEAD + 256)

/**
 * Create a object from a piece of a array.  The memory for the object will be
 * managed outside of the cps_api_object API but allows the object to be
 * stack based.  The minimum size of the array should be
 *    CPS_API_MIN_OBJ_LEN.
 @verbatim
 char buff[CPS_API_MIN_OBJ_LEN + 500];
 cps_api_object_t obj = cps_api_object_init(buff,bufflen);
 @endverbatim
 *
 * @return the object that is allocated or NULL if not possible to create
 */
cps_api_object_t cps_api_object_init(void *data, size_t bufflen);

/**
 * Allocate a cps api object
 *
 * @return a pointer to the object that is created.  The function and line
 *                 are passed in for debugging purposes
 */
#define cps_api_object_create() \
        cps_api_object_create_int(__FUNCTION__,__LINE__,__FILE__)

/**
 * Create a object - don't use directly - use CPS_API_OBJ_ALLOC macro
 *
 * @param desc the string description of the use of the object or the filename
 * @param line the file line on which the allocation exists.
 * @return the object that is allocated or NULL if not possible to create
 */
cps_api_object_t cps_api_object_create_int(const char *desc,unsigned int line, const char *name);

/**
 * Clone an object to another object
 *
 * @param dest the destination object
 * @param src the source object
 * @return true if the object has been cloned
 */
bool cps_api_object_clone(cps_api_object_t dest, cps_api_object_t src);


/**
 * This API will delete the object and remove any corresponding attributes
 * @param the object to delete
 */
void cps_api_object_delete(cps_api_object_t obj);


/**
 * Copy the key into the object
 */
void cps_api_object_set_key(cps_api_object_t obj, cps_api_key_t *key);

/**
 * Get the first matching attribute that contains the attribute id
 * @param object containing the item
 * @param attr the attribute ID to get
 * @return the attribute that is found or CPS_API_ATTR_NULL
 */
cps_api_object_attr_t cps_api_object_attr_get(cps_api_object_t obj, cps_api_attr_id_t attr);

/**
 * Search for all of the attributes listed in attr.  If any of them are missing the call fails
 * with a false.
 * @param obj the object in question
 * @param attr the list of attributes desired
 * @param pointers the list of attribute pointers
 * @param count the count of both the list of attr ids and the list of pointers
 * @return true if all of the attributes are found
 */
bool cps_api_object_attr_get_list(cps_api_object_t obj, cps_api_attr_id_t *attr, cps_api_object_attr_t *pointers, size_t count);

/**
 * Get the first matching attributes who's indexes are in the array "attr".
 * For example if the attribute IDs are numbered from 1 to 10.  Fill up the array
 * with pointers to the attributes.
 *
 * On return, all of the missing attributes are set to CPS_API_ATTR_NULL
 * while found attributes will have their pointers filled in.
 *
 * @verbatim
 *  //want to get all attributes into an array for lots of processing
 *  enum { IMP_ATTR_1=1, IMP_ATTR_2=2,... IMP_ATTR_20=20. MAX=100};
 *  cps_api_object_attr_t list[MAX];
 *  cps_api_object_attr_get_list(obj,0,list,sizeof(list)/sizeof(*list));
 *
 *  or...
 *  const int DESIRED_RANGE=10;
 *  const int START_ATTR_RANGE_ID=30;
 *  cps_api_object_attr_t list[DESIRED_RANGE];  //only care about attributes from
 *                                   //30-39
 *  cps_api_object_attr_get_list(obj,START_ATTR_RANGE_ID,list,sizeof(list)/sizeof(*list));

 *
 * @endverbatim
 *
 * @param object containing the item
 * @param base_attr_id is the first attribute in the array
 *         (eg.. all attributes >= base_attr_id and < base_attr_id+len will be
 *         put into the base_attr_id list.
 * @param attr the list of attribute ID to fill in
 * @param len the length of the list of attributes
 *
 *
 */
void cps_api_object_attr_fill_list(cps_api_object_t obj, size_t base_attr_id, cps_api_object_attr_t *attr, size_t len);

/**
 * Remove an attribute from the object.  Pass in an attribute id of the attribute to remove.
 * If the attribute is invalid the request is ignored otherwise the attr is removed.
 * @param the object that contains the attribute to be deleted
 * @param attr the attribute of the item to delete
 */
void cps_api_object_attr_delete(cps_api_object_t obj, cps_api_attr_id_t attr);

/**
 * This API is meant for embedded objects - an object that contains objects itself.
 * This API will get an embedded attribute.  The attribute can be embedded many levels
 * deep.
 *
 * @param object that contains the attribute
 * @param id a list of attribute ids (for embedded objects)
 * @param id_size the length of ids in the attribute
 * @return the cps_api_object_attr_t for the object.
 */
cps_api_object_attr_t cps_api_object_e_get(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size);

/**
 * This API is meant for getting iterators on for any attribute within the object.
 * If you query a single attribute, it will only create an iterator that point to the existing attribute.
 * If you use the cps_api_object_it_inside API, you can further go inside the attribute
 * deep.
 *
 * @param object that contains the attribute
 * @param id a list of attribute ids (for embedded objects)
 * @param id_size the length of ids in the attribute
 * @return the cps_api_object_it_t for the object.
 */
bool cps_api_object_it(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size, cps_api_object_it_t *it);

/**
 * This API is meant for embedded objects - an object that contains objects itself.
 * Users will need to specify the containment in the cps_api_attr_id_t list
 *
 * Add an attribute or embedded attribute to the object.  The attribute will be copied into the object.
 * @param object in which to add the attribute
 * @param id a list of attribute ids (for embedded objects)
 * @param id_size the length of ids in the attribute
 * @param data the data to add
 * @param len the length of attribute
 * @return true of the item is added otherwise false
 */
bool cps_api_object_e_add(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size, cps_api_object_ATTR_TYPE_t type, const void *data, size_t len);

/**
 * A helper function to add a int type to an object.  The int type can be 1, 2, 4, 8 bytes long
 *
 * @param obj the object to add the attribute to
 * @param id the attribute id list
 * @param id_size the length of the attribute list
 * @param data the data to add
 * @param len the size of the data
 * @return true if the method passed successfully otherwise a false and the object is untouched
 */
static inline bool cps_api_object_e_add_int(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size, const void *data, size_t len) {
    return cps_api_object_e_add(obj,id,id_size,cps_api_object_int_type_for_len(len),data,len);
}

/**
 * Add an attribute to the object.  The attribute will be copied into the object.
 * @param object in which to add the attribute
 * @param id of the attribute to add
 * @param data the data to add
 * @param len the length of attribute
 * @return true of the item is added otherwise false
 */
static inline bool cps_api_object_attr_add(cps_api_object_t obj, cps_api_attr_id_t id,const void *data, size_t len) {
    return cps_api_object_e_add(obj,&id,1,cps_api_object_ATTR_T_BIN,data,len);
}

/**
 * Add an attribute to the object.  The attribute will be copied into the object.
 * @param object in which to add the attribute
 * @param id of the attribute to add
 * @param data the data to add
 * @return true of the item is added otherwise false
 */
static inline bool cps_api_object_attr_add_u16(cps_api_object_t obj, cps_api_attr_id_t id,uint16_t data) {
    return cps_api_object_e_add(obj,&id,1,cps_api_object_ATTR_T_U16,&data,sizeof(data));
}

/**
 * Add an attribute to the object.  The attribute will be copied into the object.
 * @param object in which to add the attribute
 * @param id of the attribute to add
 * @param data the data to add
 * @return true of the item is added otherwise false
 */
static inline bool cps_api_object_attr_add_u32(cps_api_object_t obj, cps_api_attr_id_t id,uint32_t data) {
    return cps_api_object_e_add(obj,&id,1,cps_api_object_ATTR_T_U32,&data,sizeof(data));
}

/**
 * Add an attribute to the object.  The attribute will be copied into the object.
 * @param object in which to add the attribute
 * @param id of the attribute to add
 * @param data the data to add
 * @return true of the item is added otherwise false
 */
static inline bool cps_api_object_attr_add_u64(cps_api_object_t obj, cps_api_attr_id_t id,uint64_t data) {
    return cps_api_object_e_add(obj,&id,1,cps_api_object_ATTR_T_U64,&data,sizeof(data));
}

/**
 * Get the first attribute within the object.  This can be passed to cps_api_object_attr_next to walk through
 * the list of attributes.  For example...
 *
 @verbatim
    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    while (cps_api_object_it_valid(&it)) {

        print_attr(it.tlv);
        it = cps_api_object_attr_next(obj, it);
        if (it == NULL) {
            printf("Null\n");
        }

    }
 @endverbatim
 @param obj the object in question
 @return the first attribute in the object or CPS_API_ATTR_NULL if there are no objects
 */
void cps_api_object_it_begin(cps_api_object_t obj, cps_api_object_it_t *it);



/**
 * Print the object into a string buffer.  Debug function only
 * @param obj the object to be printed.
 * @param buff the buffer to print the object to
 * @param len the length of the buffer
 * @return the pointer to buff
 */
const char * cps_api_object_to_string(cps_api_object_t obj, char *buff, size_t len);


/**
 * Get the current object's key
 * @param obj the object in question
 * @return the pointer to the object's key
 */
cps_api_key_t * cps_api_object_key(cps_api_object_t obj);


/**
 * This API returns the length of the object as a physical array.
 * @param obj the object in question
 * @return the length of bytes of the object if converyed to an uint8_t array
 */
size_t cps_api_object_to_array_len(cps_api_object_t obj) ;

/**
 * This API returns the object's internal array.  This can be written to disk, or passed
 * to a subsequent cps_api_array_to_obect API.
 * @param obj the object in question.
 * @return a pointer to the object's internal data array
 */
void * cps_api_object_array(cps_api_object_t obj);

/**
 * Reserve space in the object to receive a future object.
 * Can be read directly into the object array.
 * @param obj to reserve space on.
 * @param amount_of_space_to_reserve is the size of buffer needed within the object
 * @return true if could reserve the space
 */
bool cps_api_object_reserve(cps_api_object_t obj, size_t amount_of_space_to_reserve);

/**
 * Query the object to determine how much space it has reserved.
 * @param obj to reserve space on.
 * @return total capacity of the object in bytes
 */
size_t cps_api_object_get_reserve_len(cps_api_object_t obj);

/**
 * Indicate to the object that it has recieved new contents due reading data from disk or
 * from reseved ram, etc..  Need to pass the amount of data received.  The object will
 * return true if the newly updated object is valid.
 *
 * @param obj that was updated
 * @param size_of_object_received the amount of space used in the object
 * @return true if the object is still valid
 */
bool cps_api_object_received(cps_api_object_t obj, size_t size_of_object_received);

/**
 * This API will convert the array back to an object.  The object must be created before
 * calling this API (therefore the obj must not be NULL).
 * @param data the data to convert to an object.
 * @param len the length of the data array
 * @param obj the object that will contain the key + values that are extracted
 * @return true if the  object is successfully extracted otherwise false
 */
bool cps_api_array_to_object(void * data, size_t len,cps_api_object_t obj) ;

/**
 * Create a list of objects.  Each object added to the list is not copied but the list does
 * provide a helper function to delete any contained objects.
 *
 * @return a pointer to the list object
 */
cps_api_object_list_t cps_api_object_list_create(void);

/**
 * Destroy the list of objects.  Optionally delete all objects that are contained by the list
 * @param list the list in question
 * @param delete_objects true if the user wants all contained objects to also be freed
 */
void cps_api_object_list_destroy(cps_api_object_list_t list, bool delete_objects);

/**
 * Add an object to the list.  The object is not copied into the list.
 * @param list to add the object to
 * @param obj the object to add
 * @return true if the object is successfully added
 */
bool cps_api_object_list_append(cps_api_object_list_t list,cps_api_object_t obj);


/**
 * Create an object and add it to the list.  If the creation of the object fails
 * return NULL.  If adding the object to the list fails return NULL
 * @param list to add the object to
 * @return the object created if successful otherwise NULLL
 */
cps_api_object_t cps_api_object_list_create_obj_and_append(cps_api_object_list_t list);

/**
 * Remove an object from the list.  The user specifies an index of the object to remove.
 * After the object is removed all previous indexes are invalid -
 * as the list will shrink
 *
 * @param list the list to remove the object from
 * @param ix the index of the item to remove
 */
void cps_api_object_list_remove(cps_api_object_list_t list,size_t ix);

/**
 * Get an object in the list using an index.
 * @param list the list in question
 * @param ix the index for the object to get
 * @return a object or CPS_API_OBJECT_NULL if no object found at that index
 */
cps_api_object_t cps_api_object_list_get(cps_api_object_list_t list,size_t ix);

/**
 * Get the number of objects contained within the list.
 * @param list the list to query
 * @return the number of objects in the list
 */
size_t cps_api_object_list_size(cps_api_object_list_t list);


/**
 * Print out the contents of the CPS API object tracker database.
 * @return true if no entries - false if there are elements
 */
bool cps_api_list_debug(void) ;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * This is an object helper.  It will take an object and provide the ability to clean it up
 * automatically once it goes out of scope.
 *
 */
class cps_api_object_guard {
    cps_api_object_t obj;
public:
    /**
     * takes a newly created object and stores
     * @param o the object to manage internally
     */
    cps_api_object_guard(cps_api_object_t o) : obj(o) {}

    /**
     * When the object goes out of scope delete the object if it is still owned  (not NULL)
     */
    ~cps_api_object_guard() {
        if (obj!=NULL) cps_api_object_delete(obj);
    }
    /**
     * Check to see if the object is valid
     */
    bool valid() { return obj!=NULL; }

    /**
     * Release the object - will not be destroyed when this cps_api_object_guard
     * is going out of scope
     */
    cps_api_object_t release() { cps_api_object_t tmp = obj; obj = NULL; return tmp; }
    /**
     * Return the contained object
     */
    cps_api_object_t get() { return obj; }

    /**
     * Free the contained object
     */
    void free() {
        if (obj!=NULL) cps_api_object_delete(obj);
        obj = NULL;
    }

    /**
     * Remove the existing object if any and then assign the guard to manage the new object
     * @param o
     */
    void set(cps_api_object_t o) {
        free();
        obj = o;
    }
};

/**
 * Cleanup after a list automatically for a user.  This just simplifies cleanup and avoids memory issues
 *
 */
class cps_api_object_list_guard {
    cps_api_object_list_t lst;
public:
    /**
     * Create the object guard passing in the object.
     * @param l the list to manage
     */
    cps_api_object_list_guard(cps_api_object_list_t l) : lst(l){}
    /**
     * Release the contained list and don't clean it on destruction
     */
    void release() { lst=NULL;}
    /**
     * Free the contained object if there is any.
     */
    void free() {
        if (lst!=NULL) cps_api_object_list_destroy(lst,true);
        lst = NULL;
    }
    /**
     * Set a new list into the object guard
     * @param l the list to monitor
     */
    void set(cps_api_object_list_t l) { free(); lst = l; }
    /**
     * Get the currently managed list
     * @return the guarded list
     */
    cps_api_object_list_t get() { return lst; }
    /**
     * Clean up when going out of scope
     */
    ~cps_api_object_list_guard() {
        free();
    }

};

#endif

#endif /* CPS_API_COMMON_LIST_H_ */

