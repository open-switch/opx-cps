/*
 * filename: db_common_list.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_common_list.h
 */

#ifndef DB_COMMON_LIST_H_
#define DB_COMMON_LIST_H_


#include "ds_common_types.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * ds_common_list_t;

/**
 * Each entry of a list will be composed of the following items.
 * An object type and an object itself.
 * Casting is required inside and outside of the API but wrapper
 * Macros  will be available to help simplify
 */
typedef struct {
    ds_object_type_t type;
    void *data;
    size_t len;
    bool allocated;
}ds_list_entry_t;

void db_list_debug();

/**
 * Create a list - don't use directly - use DB_LIST_ALLOC macro
 * @return the list that is allocated or NULL if not possible to create
 */
ds_common_list_t ds_list_create(const char *desc,unsigned int line);

/**
 * Allocate a db list structure
 * @return a pointer to the list
 */
#define DS_LIST_ALLOC \
        ds_list_create(__FUNCTION__,__LINE__)

/**
 * All lists have to be deleted after created.  The following removes any items from the created
 * list.
 * @param the list to remove
 */
void ds_list_destroy(ds_common_list_t list);

/**
 * Get an item from the list at the given index
 * @param list containing the item
 * @param ix the index of the item
 * @return the pointer to the item.  The list still owns the item.
 */
ds_list_entry_t *ds_list_elem_get(ds_common_list_t list,size_t ix);

/**
 * Remove an element from the list.  Pass in an index.  If the index is invalid the request
 * is ignored otherwise the item is removed.
 * @param the list that contains the elem to be deleted
 * @param ix the index of the item
 */
void ds_list_elem_del(ds_common_list_t list,size_t ix);

/**
 * Add an element to the list.  If the user desires, a deep copy will be made
 * @param list of elements in which to add
 * @param type the object type to add
 * @param data the data to add
 * @param len the length of item
 * @param deep_copy true if the user makes a deep copy
 * @return true of the item is added
 */
bool ds_list_elem_add(ds_common_list_t list, ds_object_type_t type,void *data, size_t len, bool deep_copy);

/**
 * get the number of elements in the list
 * @param list is the list for which the elements are added
 * @return size
 */
size_t ds_list_get_len(ds_common_list_t list);

/**
 * Return the element pointed to by index and increase it.  If this is the last element, return null
 * @param list the list being iterated over
 * @param index[out] the current index.  Set the index to 0 to start the iteration.
 *     This function will return the next valid index or NULL if there is no next
 * @return the object at the current index.
 */
ds_list_entry_t *ds_list_elem_next(ds_common_list_t list,size_t *index);

/**
 * A template the function that will calculate how much space an entry would take
 * @param application defined context
 * @param entry to calculate the size of an element
 * @return size of entry on array
 */
typedef size_t (*ds_list_elem_array_calc)(void *context,ds_list_entry_t *entry);
/**
 * Determine how much space this array would occupy
 * @param list the list to iterate over
 * @param optional_calc_fun will be used to calculate the lenght of each element if not NULL.
 *     if it is NULL, each data element will be added using the "len" field of the structure
 * @param context the context to pass to the calculate function
 * @return
 */
size_t ds_list_array_len(ds_common_list_t list,ds_list_elem_array_calc optional_calc_fun, void *context);

/**
 *Structure that is passed to the convert entry function.
 */
typedef struct {
    void * context; //! this is the ccontext passed to the db_list_mk_array function
    ds_common_list_t list; //! this is the list containing the entry
    ds_list_entry_t *entry;    //! this is the entry itself
    void *data;            //!this is where to write the data.  Must be updated to point to the next free location on return
    size_t space;    //!the remaining space - also must be updated and reduced after converting the current entry
} ds_list_convert_operation_t;
/**
 * Convert an entry to another format..
 * @param context a parameter passed to the function
 * @param list the list to operate on
 * @param entry to convert to a different format
 * @param data[out] to write to and return a pointer to the next location
 * @param space[out] reserved for the write and return amount of space remaining
 */
typedef void(*ds_list_convert_function)(ds_list_convert_operation_t *context);

/**
 * This structure is passed to the db_list_mk_array function and will perform the conversion from an element to a data field if specified
 */
typedef struct {
    ds_list_convert_function convert_fun;//! this will convert the entry
    void *convert_fun_context; //!this is the context to that function
    ds_list_elem_array_calc space_calc; //!this will determine the space needed for a field
    void *space_calc_context;    //! this is the context for the above function
}ds_list_convert_functions_t;

/**
 * Make an array from a list
 * @param list the list to convert
 * @param data the data to convert
 * @param len the length of the data reserved for the operation
 * @param converter is the structure - can be NULL and then the data will be memcpy'ed into the array directly as a TLV
 * @return true if passed otherwise fail...
 */
bool ds_list_mk_array(ds_common_list_t list,void *data, size_t len, ds_list_convert_functions_t *converter);

/**
 * convert an array of something (strings, binary, etc..) to a list
 * @param list the list to create
 * @param data the actual data to load into the list
 * @param len the length of the data
 * @param fun the function that will be used
 * @param convert_context is the context passed to the function
 * @param deep_copy is true if a deep copy of each entry is required
 * @return true if successful
 */
bool ds_list_from_array(ds_common_list_t list,void *data, size_t len, ds_list_convert_function fun,void * convert_context, bool deep_copy);


#ifdef __cplusplus
}
#endif


#endif /* DB_COMMON_LIST_H_ */

