/* OPENSOURCELICENSE */
/*
 * cps_key_internals.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_KEY_INTERNALS_H_
#define CPS_KEY_INTERNALS_H_

#include <stdint.h>
#include "endian.h"

/** @defgroup CPSAPI_Internal The CPS API Internal API
 *@warning this is an internal file.  Don't use the APIs directly.
 * this file is for internal manipulation of the key and the definition of the key
 * itself.
@{
*/
#define CPS_OBJ_KEY_BYTE_MAX_LEN (256)
/**
 * opaque key structure.  Each element in the key is a uint32_t.
 */
typedef uint8_t cps_api_key_t[CPS_OBJ_KEY_BYTE_MAX_LEN];



#define CPS_OBJ_KEY_ELEM_SIZE (sizeof(uint32_t))
#define CPS_OBJ_KEY_LEN_POS (0)
#define CPS_OBJ_KEY_ATTR_POS (1)

#define CPS_OBJ_KEY_ELEM_START (CPS_OBJ_KEY_ATTR_POS+1)
#define CPS_OBJ_KEY_HEADER_SIZE (CPS_OBJ_KEY_ELEM_START * CPS_OBJ_KEY_ELEM_SIZE)

/**
 * Get the length in bytes of the key
 */
static inline uint32_t cps_api_key_get_len_in_bytes(cps_api_key_t * elem) {
    return htole32(((uint32_t*)elem)[CPS_OBJ_KEY_LEN_POS]) * CPS_OBJ_KEY_ELEM_SIZE;
}

/**
 * @}
 */
#endif /* CPS_KEY_INTERNALS_H_ */
