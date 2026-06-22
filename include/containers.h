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

/// @defgroup array Dynamic arrays
/// @{

/// @defgroup array_allocation Allocation 
/// @ingroup array
/**
 * @brief Heap-allocates a new array.
 * @param elem_size Size in bytes of the elements of the array. Must be > 0.
 * @param cap Initial capacity (number of elements) of the array. Must be > 0.
 * @param err Optional output error information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return A pointer to the new heap-allocated array. NULL on failure.
 * @ingroup array_allocation
 */
array_t* array_create(size_t elem_size, size_t cap, error_t* err);

/**
 * @brief Destroys the array and frees its resources.
 * @param arr The array to destroy. It can be NULL.
 * @ingroup array_allocation
 */
void array_destroy(array_t* arr);

/// @} // array_allocation

/// @defgroup array_access Element access
/// @ingroup array

/**
 * @brief Access specified element with bounds checking.
 *
 * An empty array is not considered an error if the input index is zero, it just returns NULL.
 *
 * @param arr The array. Must be not NULL.
 * @param index Index of element to access. Index 0 is valid againts empty arrays.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return A pointer to the element, NULL on failure (or empty array if given index is 0).
 * @ingroup array_access
 */
void* array_at(array_t* arr, size_t index, error_t* err);

/**
 * @brief Access the first element of the array.
 *
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return A pointer to the element, NULL on failure (or empty array).
 * @ingroup array_access
 */
void* array_front(array_t* arr, error_t* err);

/**
 * @brief Access the last element of the array.
 *
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return A pointer to the element, NULL on failure (or empty array).
 * @ingroup array_access
 */
void* array_back(array_t* arr, error_t* err);

/// @} // array_access

/// @defgroup array_modifiers Modifiers
/// @ingroup array

/**
 * @brief Adds an element to the end of the array.
 * @param arr The array. Must be not NULL.
 * @param data The item to push into the array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_modifiers
 */
void array_push_back(array_t* arr, void* data, error_t* err);

/// @} // array_modifiers

/// @} // array

#endif // H_CONTAINERS
