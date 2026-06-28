/// @file containers.h
#ifndef H_CONTAINERS
#define H_CONTAINERS

#include <stdlib.h>
#include <stdbool.h>

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
    ERROR_INVALID_STATE,  ///< Internal state is corrupted; operation cannot proceed.
} error_e;

/**
 * @brief Generic error you can use to understand why something is failing.
 */
typedef struct {
    error_e code; ///< Type of error. @see error_e 
    char msg[256]; ///< Verbose message from the error.
} error_t;

/// @defgroup array Dynamic arrays
/// @{

/**
 * @brief Generic dynamic array.
 */
typedef struct {
    void* data; ///< pointer to the internal data of the array.
    size_t cap; ///< current capacity of the array.
    size_t count; ///< current number of elements in the array.
    size_t elem_size; ///< size in bytes of a single element of the array.
} array_t;

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
 * @param arr The array. Must be not NULL and not empty.
 * @param index Index of element to access. It must be < count.
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

/// @defgroup array_capacity Capacity
/// @ingroup array

/**
 * @brief Checks whether the container is empty.
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return True if the array is empty. False otherwise, or on failures.
 * @ingroup array_capacity
 */
bool array_is_empty(array_t* arr, error_t* err);

/**
 * @brief Returns the maximum possible number of elements.
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Maximum possible number of elements for the given array. Zero on failures.
 * @ingroup array_capacity
 */
size_t array_max_size(array_t* arr, error_t* err);

/**
 * @brief Reserves storage for the array.
 * @param arr The array. Must be not NULL.
 * @param to_reserve Capacity to reserve. It must not exceed maximum number of elements.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_capacity
 */
void array_reserve(array_t* arr, size_t to_reserve, error_t* err);

/**
 * @brief Reduces memory usage by freeing unused memory.
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_capacity
 */
void array_shrink_to_fit(array_t* arr, error_t* err);


/// @} // array_capacity

/// @defgroup array_modifiers Modifiers
/// @ingroup array

/**
 * @brief Clears the contents of the array.
 * @param arr The array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_modifiers
 */
void array_clear(array_t* arr, error_t* err);

/**
 * @brief Inserts an element into the array.
 * @param arr The array. Must be not NULL.
 * @param index Position to insert the element in. It must be <= count.
 * @param item Item's memory to insert in the array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_modifiers
 */
void array_insert(array_t* arr, size_t index, const void* item, error_t* err);

/**
 * @brief Adds an element to the end of the array.
 * @param arr The array. Must be not NULL.
 * @param item The item to push into the array. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_modifiers
 */
void array_push_back(array_t* arr, const void* item, error_t* err);

/**
 * @brief Erases an element from the array.
 * @param arr The array. Must be not NULL.
 * @param index The index of the element to erase. It must be < count.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup array_modifiers
 */
void array_erase(array_t* arr, size_t index, error_t* err);

/// @} // array_modifiers

/// @} // array

/// @defgroup list Linked lists
/// @{

// Forward declaration
typedef struct list_node list_node_t;

/**
 * @brief Generic linked list.
 */
typedef struct list {
    list_node_t* root; ///< pointer to the root node.
    list_node_t* tail; ///< pointer to the tail node.
    size_t count;      ///< current number of nodes in the list.
    size_t elem_size;  ///< Size in bytes of each element stored in the list.
} list_t;

/// @defgroup list_allocation Allocation
/// @ingroup list

/**
 * @brief Creates a new linked list.
 * @param elem_size Size of the data referenced by the nodes of the list. Must be > 0.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return pointer to the newly created list. NULL on failure.
 * @ingroup list_allocation
 */
list_t* list_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the list and frees its resources.
 * @param list The list to destroy. It can be NULL.
 * @ingroup list_allocation
 */
void list_destroy(list_t* list);

/// @} // list_allocation

/// @defgroup list_capacity Capacity
/// @ingroup list

/**
 * @brief Checks whether the container is empty.
 * @param list The list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return True if the list is empty. False otherwise, or on failures.
 * @ingroup list_capacity
 */
bool list_is_empty(list_t* list, error_t* err);

/// @} // list_capacity

/// @defgroup list_modifiers Modifiers
/// @ingroup list

/**
 * @brief Clears the contents of the list.
 * @param list The list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup list_modifiers
 */
void list_clear(list_t* list, error_t* err);

/**
 * @brief Adds an element to the end of the list.
 * @param list The list. Must be not NULL.
 * @param item The item to push into the list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup list_modifiers
 */
void list_push_back(list_t* list, const void* item, error_t* err);

/**
 * @brief Adds an element to the start of the list.
 * @param list The list. Must be not NULL.
 * @param item The item to push into the list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup list_modifiers
 */
void list_push_front(list_t* list, const void* item, error_t* err);

/**
 * @brief Removes the node at the end of the list.
 * @param list The list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @param item_out If non-NULL, the removed node's data gets copied here.
 * @ingroup list_modifiers
 */
void list_pop_back(list_t* list, void* item_out, error_t* err);

/**
 * @brief Removes the node at the start of the list.
 * @param list The list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @param item_out If non-NULL, the removed node's data gets copied here.
 * @ingroup list_modifiers
 */
void list_pop_front(list_t* list, void* item_out, error_t* err);

/**
 * @brief Inserts a new element before the specified position.
 *
 * The new node is inserted immediately before `pos`.
 *
 * @param list The list. Must be not NULL.
 * @param pos Iterator (node) indicating the insertion position.
 *            Must be either a valid node in the list or NULL.
 *            If NULL, the element is inserted at the end of the list.
 * @param item Pointer to the data to insert. Must be not NULL.
 *             The data is copied using `elem_size`.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @note Complexity: O(1) if inserting at known node.
 * @note Invalid `pos` (not belonging to `list`) results in undefined behavior.
 * @ingroup list_modifiers
 */
void list_insert_before(list_t* list, list_node_t* pos, const void* item, error_t* err);

/**
 * @brief Inserts a new element after the specified position.
 *
 * The new node is inserted immediately after `pos`.
 *
 * @param list The list. Must be not NULL.
 * @param pos Iterator (node) indicating the insertion position.
 *            Must be a valid node in the list or NULL.
 *            If NULL, the element is inserted at the front of the list.
 * @param item Pointer to the data to insert. Must be not NULL.
 *             The data is copied using `elem_size`.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @note Complexity: O(1) if inserting at known node.
 * @note If `pos` is NULL, insertion happens at the front of the list.
 * @note Invalid `pos` (not belonging to `list`) results in undefined behavior.
 * @ingroup list_modifiers
 */
void list_insert_after(list_t* list, list_node_t* pos, const void* item, error_t* err);

/**
 * @brief Removes a node from the list.
 *
 * The node is detached from the list and its memory is freed.
 *
 * @param list The list. Must be not NULL.
 * @param node Node to remove. Must be a valid node belonging to `list`.
 * @param item_out Optional output buffer. If non-NULL, the removed element's
 *                 data is copied into it before the node is freed.
 *                 Must be large enough to hold `elem_size` bytes.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @note Complexity: O(1)
 * @note After this call, `node` becomes invalid and must not be used.
 * @note Invalid `node` (not belonging to `list`) results in undefined behavior.
 * @ingroup list_modifiers
 */
void list_erase(list_t* list, list_node_t* node, void* item_out, error_t* err);

/// @} // list_modifiers

/// @defgroup list_iteration Iteration
/// @ingroup list

/**
 * @brief Returns an iterator to the first node of the list.
 *
 * @param list The list to iterate. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @return Pointer to the first node in the list, or NULL if the list is empty
 *         or on failure.
 *
 * @note This function does not modify the list.
 * @ingroup list_iteration
 */
list_node_t* list_begin(list_t* list, error_t* err);

/**
 * @brief Returns the end iterator for the list.
 *
 * The end iterator represents a position past the last element and is
 * used as a termination condition during iteration.
 *
 * @param list The list. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @return Always returns NULL (end sentinel). Provided for API symmetry and
 *         future extensibility.
 *
 * @note This function does not modify the list.
 * @ingroup list_iteration
 */
list_node_t* list_end(list_t* list, error_t* err);

/**
 * @brief Returns the next node in the list.
 *
 * @param node Current node. Must be a valid node belonging to a list and not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @return Pointer to the next node, or NULL if the end of the list is reached
 *         or on failure.
 *
 * @note This function does not modify the list.
 * @ingroup list_iteration
 */
list_node_t* list_next(list_node_t* node, error_t* err);

/**
 * @brief Returns the previous node in the list.
 *
 * @param node Current node. Must be a valid node belonging to a list and not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 *
 * @return Pointer to the previous node, or NULL if the beginning of the list is
 *         reached or on failure.
 *
 * @note This function does not modify the list.
 * @ingroup list_iteration
 */
list_node_t* list_prev(list_node_t* node, error_t* err);

/**
 * @brief Returns a pointer to the data stored in a node.
 * @param node The node. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Pointer to the node's data, or NULL on failure.
 * @ingroup list_iteration
 */
void* list_node_data(list_node_t* node, error_t* err);

/// @} // list_iteration
    
/// @} // list

///@defgroup queue Queues
///@ {

/**
 * @brief Generic queue.
 * @ingroup queue
 */
typedef struct {
    list_t* list; ///< Internal container.
} queue_t;

/// @defgroup queue_allocation Allocation
/// @ingroup queue
/// @{

/**
 * @brief Creates a new queue.
 * @param elem_size Size in bytes of the elements stored in the queue.
 *                  Must be greater than zero.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Pointer to the newly created queue. NULL on failure.
 * @ingroup queue_allocation
 */
queue_t* queue_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the queue and frees its resources.
 * @param queue The queue to destroy. It can be NULL.
 * @ingroup queue_allocation
 */
void queue_destroy(queue_t* queue);

/// @} // queue_allocation

/// @defgroup queue_capacity Capacity
/// @ingroup queue
/// @{

/**
 * @brief Checks whether the queue is empty.
 * @param queue The queue. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return True if the queue is empty. False otherwise, or on failure.
 * @ingroup queue_capacity
 */
bool queue_is_empty(queue_t* queue, error_t* err);

/**
 * @brief Returns the number of elements in the queue.
 * @param queue The queue. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Current number of elements in the queue. Zero on failures.
 * @ingroup queue_capacity
 */
size_t queue_size(queue_t* queue, error_t* err);

/// @} // queue_capacity

/// @defgroup queue_access Element access
/// @ingroup queue
/// @{

/**
 * @brief Returns a pointer to the element at the front of the queue.
 * @param queue The queue. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Pointer to the front element, or NULL if the queue is empty or on
 *         failure.
 * @note The returned pointer remains valid until the element is removed or the
 *       queue is modified in a way that invalidates it.
 * @ingroup queue_access
 */
void* queue_front(queue_t* queue, error_t* err);

/**
 * @brief Returns a pointer to the element at the back of the queue.
 * @param queue The queue. Must be not NULL.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @return Pointer to the back element, or NULL if the queue is empty or on
 *         failure.
 * @note The returned pointer remains valid until the element is removed or the
 *       queue is modified in a way that invalidates it.
 * @ingroup queue_access
 */
void* queue_back(queue_t* queue, error_t* err);

/// @} // queue_access

/// @defgroup queue_modifiers Modifiers
/// @ingroup queue
/// @{

/**
 * @brief Inserts an element at the back of the queue.
 *
 * @param queue The queue. Must be not NULL.
 * @param item Pointer to the element to insert. Must be not NULL.
 *             The element is copied into the queue.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup queue_modifiers
 */
void queue_push(queue_t* queue, const void* item, error_t* err);

/**
 * @brief Removes the element at the front of the queue.
 *
 * @param queue The queue. Must be not NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed element is
 *                 copied here before being removed. The buffer must be large
 *                 enough to hold `elem_size` bytes.
 * @param err Optional error output information. If non-NULL and the call fails,
 *            it will contain details about the failure.
 * @ingroup queue_modifiers
 */
void queue_pop(queue_t* queue, void* item_out, error_t* err);

/// @} // queue_modifiers
/// @} // queue

#endif // H_CONTAINERS
