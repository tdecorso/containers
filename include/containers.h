/// @file containers.h
#ifndef H_CONTAINERS
#define H_CONTAINERS

#include <stdlib.h>

/**
 * @brief Error codes returned by the API.
 *
 * These values indicate failure conditions and define how the caller
 * should react.
 */
typedef enum {
    ERROR_OK,             ///< No errors detected.
    ERROR_INVALID_ARGS,   ///< One or more arguments are invalid; caller must fix input before retrying. 
    ERROR_OUT_OF_MEMORY,  ///< Memory allocation failed; operation cannot proceed, retry may succeed if memory is freed.
} error_e;

/**
 * @brief Generic error you can use to understand why something is failing.
 */
typedef struct {
    error_e code; ///< Type of error. @see error_e 
    char msg[256]; ///< Verbose message from the error.
} error_t;

/**
 * @brief Generic dynamic array.
 */
typedef struct {
    void* data; ///< pointer to the internal data of the array.
    size_t cap; ///< current capacity of the array.
    size_t count; ///< current number of elements in the array.
    size_t elem_size; ///< size in bytes of a single element of the array.
} array_t;

/**
 * @defgroup array Dynamic array
 */

/**
 * @brief Heap-allocates a new array.
 * @param elem_size Size in bytes of the elements of the array.
 * @param cap Initial capacity (number of elements) of the array.
 * @param err Optional output error information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return a pointer to the new heap-allocated array. NULL on failure.
 * @ingroup array
 */
array_t* array_create(size_t elem_size, size_t cap, error_t* err);

/**
 * @brief Destroys the array and frees its resources.
 * It is safe to use on NULL pointers.
 * @param arr the array to destroy.
 * @ingroup array
 */
void array_destroy(array_t* arr);

#endif // H_CONTAINERS
