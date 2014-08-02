/** OPENSOURCELICENSE */
/*
 * db_common_list.h
 */

#ifndef DB_COMMON_LIST_H_
#define DB_COMMON_LIST_H_

#include "db_common_types.h"

#include <stddef.h>
#include <stdbool.h>

typedef void * db_common_list_t;

/**
 * Each entry of a list will be composed of the following items.
 * An object type and an object itself.
 * Casting is required inside and outside of the API but wrapper
 * Macros  will be available to help simplify
 */
typedef struct {
    db_object_type_t type;
    void *data;
    size_t len;
    bool allocated;
}db_list_entry_t;

void db_list_debug();


/**
 * Create a list - don't use directly - use DB_LIST_ALLOC macro
 * @return the list that is allocated or NULL if not possible to create
 */
db_common_list_t db_list_create(const char *desc,unsigned int line);

/**
 * Allocate a db list structure
 * @return a pointer to the list
 */
#define DB_LIST_ALLOC \
        db_list_create(__FUNCTION__,__LINE)

/**
 * All lists have to be deleted after created.  The following removes any items from the created
 * list.
 * @param the list to remove
 */
void db_list_destroy(db_common_list_t list);

/**
 * Get an item from the list at the given index
 * @param list containing the item
 * @param ix the index of the item
 * @return the pointer to the item.  The list still owns the item.
 */
db_list_entry_t *db_list_elem_get(db_common_list_t list,size_t ix);

/**
 * Remove an element from the list.  Pass in an index.  If the index is invalid the request
 * is ignored otherwise the item is removed.
 * @param the list that contains the elem to be deleted
 * @param ix the index of the item
 */
void db_list_elem_del(db_common_list_t list,size_t ix);

/**
 * Add an element to the list.  If the user desires, a deep copy will be made
 * @param list of elements in which to add
 * @param type the object type to add
 * @param data the data to add
 * @param len the length of item
 * @param deep_copy true if the user makes a deep copy
 * @return true of the item is added
 */
bool db_list_elem_add(db_common_list_t list, db_object_type_t type,void *data, size_t len, bool deep_copy);

/**
 * get the number of elements in the list
 * @param list is the list for which the elements are added
 * @return size
 */
size_t db_list_get_len(db_common_list_t list);

/**
 * Return the element pointed to by index and increase it.  If this is the last element, return null
 * @param list the list being iterated over
 * @param index[out] the current index.  Set the index to 0 to start the iteration.
 *     This function will return the next valid index or NULL if there is no next
 * @return the object at the current index.
 */
db_list_entry_t *db_list_elem_next(db_common_list_t list,size_t *index);


#endif /* DB_COMMON_LIST_H_ */

