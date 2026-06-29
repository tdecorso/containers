/// @file containers.h
/// @brief Generic container library: dynamic arrays, linked lists, queues,
///        stacks, hash maps, and n-ary trees.
///
/// All containers are generic (operate on `void*` with caller-supplied
/// element sizes) and report errors through an optional `error_t` output
/// parameter. Passing NULL for `err` is always safe — errors are silently
/// swallowed.
///
/// @note This library is not thread-safe. External synchronization is required
///       for concurrent access.

#ifndef H_CONTAINERS
#define H_CONTAINERS

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Error codes returned by the API.
 *
 * These values indicate failure conditions and define how the caller
 * should react. Every API function that can fail accepts an optional
 * `error_t*` output parameter; pass NULL to ignore errors.
 */
typedef enum {
    ERROR_OK,            ///< Operation completed successfully.
    ERROR_INVALID_ARGS,  ///< One or more arguments are invalid. Fix the input before retrying.
    ERROR_OUT_OF_MEMORY, ///< Memory allocation failed. Retry may succeed after freeing memory.
    ERROR_INVALID_STATE, ///< Internal state is corrupted. The container must not be used further.
} error_e;

/**
 * @brief Carries error information out of a failing API call.
 *
 * Pass a pointer to a stack-allocated `error_t` to any API function.
 * On failure the function fills both fields. On success `code` is set
 * to `ERROR_OK`. Passing NULL is always safe.
 */
typedef struct {
    error_e code;   ///< Identifies the failure category. @see error_e
    char msg[256];  ///< Human-readable description of the failure.
} error_t;

/// @defgroup array Dynamic arrays
/// @brief Resizable contiguous buffer with O(1) random access.
/// @{

/**
 * @brief Generic dynamic array.
 *
 * Maintains a heap-allocated contiguous buffer that grows automatically
 * as elements are added. Invariant: `count <= cap` at all times.
 */
typedef struct {
    void*  data;      ///< Pointer to the contiguous element buffer.
    size_t cap;       ///< Number of elements the buffer can currently hold.
    size_t count;     ///< Number of elements currently stored. Always `<= cap`.
    size_t elem_size; ///< Size in bytes of a single element.
} array_t;

/// @defgroup array_allocation Allocation
/// @ingroup array
/// @{

/**
 * @brief Creates a new heap-allocated dynamic array.
 *
 * @param elem_size Size in bytes of each element. Must be > 0.
 * @param cap       Initial capacity in number of elements. Must be > 0.
 * @param err       Optional error output. Populated on failure.
 * @return Pointer to the new array, or NULL on failure.
 * @ingroup array_allocation
 */
array_t* array_create(size_t elem_size, size_t cap, error_t* err);

/**
 * @brief Destroys the array and frees all associated memory.
 *
 * @param arr The array to destroy. NULL is safe and does nothing.
 * @ingroup array_allocation
 */
void array_destroy(array_t* arr);

/// @} // array_allocation

/// @defgroup array_access Element access
/// @ingroup array
/// @{

/**
 * @brief Returns a pointer to the element at the given index.
 *
 * Performs bounds checking against `cap` (not `count`), so sparsely
 * inserted elements beyond `count` are reachable.
 *
 * @param arr   The array. Must not be NULL.
 * @param index Index of the element to access. Must be < cap.
 * @param err   Optional error output. Populated on failure.
 * @return Pointer to the element, or NULL if the array is empty and
 *         index is 0, or if index is out of bounds.
 * @ingroup array_access
 */
void* array_at(array_t* arr, size_t index, error_t* err);

/**
 * @brief Returns a pointer to the first element.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @return Pointer to the first element, or NULL if the array is empty.
 * @ingroup array_access
 */
void* array_front(array_t* arr, error_t* err);

/**
 * @brief Returns a pointer to the last element.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @return Pointer to the last element, or NULL if the array is empty.
 * @ingroup array_access
 */
void* array_back(array_t* arr, error_t* err);

/// @} // array_access

/// @defgroup array_capacity Capacity
/// @ingroup array
/// @{

/**
 * @brief Returns true if the array contains no elements.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @return true if `count == 0`, false otherwise or on failure.
 * @ingroup array_capacity
 */
bool array_is_empty(array_t* arr, error_t* err);

/**
 * @brief Returns the theoretical maximum number of elements the array
 *        could ever hold, based on `elem_size` and `SIZE_MAX`.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @return Maximum element count, or 0 on failure.
 * @ingroup array_capacity
 */
size_t array_max_size(array_t* arr, error_t* err);

/**
 * @brief Ensures the array has capacity for at least `to_reserve` elements.
 *
 * If the current capacity already meets or exceeds `to_reserve`, this
 * function does nothing. Otherwise the internal buffer is reallocated.
 *
 * @param arr        The array. Must not be NULL.
 * @param to_reserve Minimum capacity to guarantee. Must not exceed
 *                   the value returned by `array_max_size`.
 * @param err        Optional error output. Populated on failure.
 * @ingroup array_capacity
 */
void array_reserve(array_t* arr, size_t to_reserve, error_t* err);

/**
 * @brief Reduces allocated memory to fit the current element count.
 *
 * Reallocates the internal buffer so that `cap == count`. If the array
 * is empty the buffer may be freed entirely.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @ingroup array_capacity
 */
void array_shrink_to_fit(array_t* arr, error_t* err);

/// @} // array_capacity

/// @defgroup array_modifiers Modifiers
/// @ingroup array
/// @{

/**
 * @brief Removes all elements from the array without freeing the buffer.
 *
 * After this call `count == 0` and `cap` is unchanged.
 *
 * @param arr The array. Must not be NULL.
 * @param err Optional error output. Populated on failure.
 * @ingroup array_modifiers
 */
void array_clear(array_t* arr, error_t* err);

/**
 * @brief Inserts an element at the given index, shifting subsequent
 *        elements one position to the right.
 *
 * If `index == count` the element is appended (equivalent to
 * `array_push_back`). The array grows automatically if needed.
 *
 * @param arr   The array. Must not be NULL.
 * @param index Insertion position. Must be <= count.
 * @param item  Pointer to the data to copy in. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @ingroup array_modifiers
 */
void array_insert(array_t* arr, size_t index, const void* item, error_t* err);

/**
 * @brief Appends an element to the end of the array.
 *
 * The array grows automatically if `count == cap`.
 *
 * @param arr  The array. Must not be NULL.
 * @param item Pointer to the data to copy in. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @ingroup array_modifiers
 */
void array_push_back(array_t* arr, const void* item, error_t* err);

/**
 * @brief Removes the last element from the array.
 *
 * @param arr      The array. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before removal. Must be large
 *                 enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the array is empty.
 * @ingroup array_modifiers
 */
void array_pop_back(array_t* arr, void* item_out, error_t* err);

/**
 * @brief Removes the first element from the array, shifting all
 *        subsequent elements one position to the left.
 *
 * @param arr      The array. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before removal. Must be large
 *                 enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the array is empty.
 * @ingroup array_modifiers
 */
void array_pop_front(array_t* arr, void* item_out, error_t* err);

/**
 * @brief Removes the element at the given index, shifting subsequent
 *        elements one position to the left.
 *
 * @param arr   The array. Must not be NULL.
 * @param index Index of the element to remove. Must be < count.
 * @param err   Optional error output. Populated on failure.
 *              Sets `ERROR_INVALID_ARGS` if index >= count.
 * @ingroup array_modifiers
 */
void array_erase(array_t* arr, size_t index, error_t* err);

/// @} // array_modifiers
/// @} // array

/// @defgroup list Linked lists
/// @brief Doubly-linked list with O(1) insertion and removal at any node.
/// @{

typedef struct list_node list_node_t; ///< Opaque linked list node.

/**
 * @brief Doubly-linked list.
 *
 * Nodes are heap-allocated and owned by the list. Each node stores a
 * copy of the caller's data (`elem_size` bytes).
 */
typedef struct list {
    list_node_t* root;      ///< First node, or NULL if the list is empty.
    list_node_t* tail;      ///< Last node, or NULL if the list is empty.
    size_t       count;     ///< Number of nodes currently in the list.
    size_t       elem_size; ///< Size in bytes of each element.
} list_t;

/// @defgroup list_allocation Allocation
/// @ingroup list
/// @{

/**
 * @brief Creates a new empty linked list.
 *
 * @param elem_size Size in bytes of each element. Must be > 0.
 * @param err       Optional error output. Populated on failure.
 * @return Pointer to the new list, or NULL on failure.
 * @ingroup list_allocation
 */
list_t* list_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the list and frees all nodes and associated memory.
 *
 * @param list The list to destroy. NULL is safe and does nothing.
 * @ingroup list_allocation
 */
void list_destroy(list_t* list);

/// @} // list_allocation

/// @defgroup list_capacity Capacity
/// @ingroup list
/// @{

/**
 * @brief Returns true if the list contains no elements.
 *
 * @param list The list. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return true if `count == 0`, false otherwise or on failure.
 * @ingroup list_capacity
 */
bool list_is_empty(list_t* list, error_t* err);

/// @} // list_capacity

/// @defgroup list_modifiers Modifiers
/// @ingroup list
/// @{

/**
 * @brief Removes all elements from the list and frees all nodes.
 *
 * After this call `count == 0` and both `root` and `tail` are NULL.
 * Calling on an already-empty list is safe and sets `ERROR_OK`.
 *
 * @param list The list. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @ingroup list_modifiers
 */
void list_clear(list_t* list, error_t* err);

/**
 * @brief Appends an element to the end of the list.
 *
 * @param list The list. Must not be NULL.
 * @param item Pointer to the data to copy in. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @ingroup list_modifiers
 */
void list_push_back(list_t* list, const void* item, error_t* err);

/**
 * @brief Prepends an element to the front of the list.
 *
 * @param list The list. Must not be NULL.
 * @param item Pointer to the data to copy in. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @ingroup list_modifiers
 */
void list_push_front(list_t* list, const void* item, error_t* err);

/**
 * @brief Removes the last element from the list.
 *
 * @param list     The list. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before the node is freed.
 *                 Must be large enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the list is empty.
 * @ingroup list_modifiers
 */
void list_pop_back(list_t* list, void* item_out, error_t* err);

/**
 * @brief Removes the first element from the list.
 *
 * @param list     The list. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before the node is freed.
 *                 Must be large enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the list is empty.
 * @ingroup list_modifiers
 */
void list_pop_front(list_t* list, void* item_out, error_t* err);

/**
 * @brief Inserts a new element immediately before `pos`.
 *
 * @param list The list. Must not be NULL.
 * @param pos  Node before which to insert. If NULL, the element is
 *             appended to the end of the list.
 * @param item Pointer to the data to copy in. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 *
 * @note Complexity: O(1).
 * @warning Passing a `pos` that does not belong to `list` is undefined behavior.
 * @ingroup list_modifiers
 */
void list_insert_before(list_t* list, list_node_t* pos, const void* item, error_t* err);

/**
 * @brief Inserts a new element immediately after `pos`.
 *
 * @param list The list. Must not be NULL.
 * @param pos  Node after which to insert. If NULL, the element is
 *             prepended to the front of the list.
 * @param item Pointer to the data to copy in. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 *
 * @note Complexity: O(1).
 * @warning Passing a `pos` that does not belong to `list` is undefined behavior.
 * @ingroup list_modifiers
 */
void list_insert_after(list_t* list, list_node_t* pos, const void* item, error_t* err);

/**
 * @brief Removes a node from the list and frees it.
 *
 * @param list     The list. Must not be NULL.
 * @param node     Node to remove. Must not be NULL and must belong to `list`.
 * @param item_out Optional output buffer. If non-NULL, the element data
 *                 is copied here before the node is freed. Must be large
 *                 enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *
 * @note Complexity: O(1).
 * @warning After this call `node` is freed and must not be dereferenced.
 * @warning Passing a `node` that does not belong to `list` is undefined behavior.
 * @ingroup list_modifiers
 */
void list_erase(list_t* list, list_node_t* node, void* item_out, error_t* err);

/// @} // list_modifiers

/// @defgroup list_iteration Iteration
/// @ingroup list
/// @{

/**
 * @brief Returns the first node of the list (begin iterator).
 *
 * Typical iteration pattern:
 * @code
 * for (list_node_t* n = list_begin(list, &err);
 *      n != list_end(list, &err);
 *      n = list_next(n, &err)) { ... }
 * @endcode
 *
 * @param list The list. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the first node, or NULL if the list is empty.
 * @ingroup list_iteration
 */
list_node_t* list_begin(list_t* list, error_t* err);

/**
 * @brief Returns the past-the-end sentinel (end iterator).
 *
 * Always returns NULL. Provided for API symmetry with `list_begin`
 * and to allow future extensibility without breaking call sites.
 *
 * @param list The list. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Always NULL.
 * @ingroup list_iteration
 */
list_node_t* list_end(list_t* list, error_t* err);

/**
 * @brief Advances to the next node.
 *
 * @param node The current node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the next node, or NULL if `node` is the last node.
 *         Reaching the end is not an error; `ERROR_OK` is set.
 * @ingroup list_iteration
 */
list_node_t* list_next(list_node_t* node, error_t* err);

/**
 * @brief Retreats to the previous node.
 *
 * @param node The current node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the previous node, or NULL if `node` is the first node.
 *         Reaching the beginning is not an error; `ERROR_OK` is set.
 * @ingroup list_iteration
 */
list_node_t* list_prev(list_node_t* node, error_t* err);

/**
 * @brief Returns a pointer to the data stored inside a node.
 *
 * The returned pointer is valid until the node is removed or the list
 * is destroyed.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the node's data buffer, or NULL on failure.
 * @ingroup list_iteration
 */
void* list_node_data(list_node_t* node, error_t* err);

/// @} // list_iteration
/// @} // list

/// @defgroup queue Queues
/// @brief FIFO queue backed by a linked list.
/// @{

/**
 * @brief Generic FIFO queue.
 *
 * Elements are pushed to the back and popped from the front.
 * Backed internally by a `list_t`.
 */
typedef struct {
    list_t* list; ///< Internal linked list storage.
} queue_t;

/// @defgroup queue_allocation Allocation
/// @ingroup queue
/// @{

/**
 * @brief Creates a new empty queue.
 *
 * @param elem_size Size in bytes of each element. Must be > 0.
 * @param err       Optional error output. Populated on failure.
 * @return Pointer to the new queue, or NULL on failure.
 * @ingroup queue_allocation
 */
queue_t* queue_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the queue and frees all associated memory.
 *
 * @param queue The queue to destroy. NULL is safe and does nothing.
 * @ingroup queue_allocation
 */
void queue_destroy(queue_t* queue);

/// @} // queue_allocation

/// @defgroup queue_capacity Capacity
/// @ingroup queue
/// @{

/**
 * @brief Returns true if the queue contains no elements.
 *
 * @param queue The queue. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return true if empty, false otherwise or on failure.
 * @ingroup queue_capacity
 */
bool queue_is_empty(queue_t* queue, error_t* err);

/**
 * @brief Returns the number of elements in the queue.
 *
 * @param queue The queue. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return Element count, or 0 on failure.
 * @ingroup queue_capacity
 */
size_t queue_size(queue_t* queue, error_t* err);

/// @} // queue_capacity

/// @defgroup queue_access Element access
/// @ingroup queue
/// @{

/**
 * @brief Returns a pointer to the front element without removing it.
 *
 * @param queue The queue. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return Pointer to the front element, or NULL if empty or on failure.
 * @note The pointer is invalidated when the element is removed or the
 *       queue is otherwise modified.
 * @ingroup queue_access
 */
void* queue_front(queue_t* queue, error_t* err);

/**
 * @brief Returns a pointer to the back element without removing it.
 *
 * @param queue The queue. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return Pointer to the back element, or NULL if empty or on failure.
 * @note The pointer is invalidated when the element is removed or the
 *       queue is otherwise modified.
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
 * @param queue The queue. Must not be NULL.
 * @param item  Pointer to the data to copy in. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @ingroup queue_modifiers
 */
void queue_push(queue_t* queue, const void* item, error_t* err);

/**
 * @brief Removes the element at the front of the queue.
 *
 * @param queue    The queue. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before removal. Must be large
 *                 enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the queue is empty.
 * @ingroup queue_modifiers
 */
void queue_pop(queue_t* queue, void* item_out, error_t* err);

/// @} // queue_modifiers
/// @} // queue

/// @defgroup stack Stacks
/// @brief LIFO stack backed by a dynamic array.
/// @{

/**
 * @brief Generic LIFO stack.
 *
 * Elements are pushed and popped from the top. Backed internally by
 * an `array_t`, giving amortized O(1) push and pop.
 */
typedef struct {
    array_t* arr; ///< Internal dynamic array storage.
} stack_t;

/// @defgroup stack_allocation Allocation
/// @ingroup stack
/// @{

/**
 * @brief Creates a new empty stack.
 *
 * @param elem_size Size in bytes of each element. Must be > 0.
 * @param err       Optional error output. Populated on failure.
 * @return Pointer to the new stack, or NULL on failure.
 * @ingroup stack_allocation
 */
stack_t* stack_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the stack and frees all associated memory.
 *
 * @param stack The stack to destroy. NULL is safe and does nothing.
 * @ingroup stack_allocation
 */
void stack_destroy(stack_t* stack);

/// @} // stack_allocation

/// @defgroup stack_capacity Capacity
/// @ingroup stack
/// @{

/**
 * @brief Returns true if the stack contains no elements.
 *
 * @param stack The stack. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return true if empty, false otherwise or on failure.
 * @ingroup stack_capacity
 */
bool stack_is_empty(stack_t* stack, error_t* err);

/**
 * @brief Returns the number of elements in the stack.
 *
 * @param stack The stack. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return Element count, or 0 on failure.
 * @ingroup stack_capacity
 */
size_t stack_size(stack_t* stack, error_t* err);

/// @} // stack_capacity

/// @defgroup stack_access Element access
/// @ingroup stack
/// @{

/**
 * @brief Returns a pointer to the top element without removing it.
 *
 * @param stack The stack. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @return Pointer to the top element, or NULL if empty or on failure.
 * @note The pointer is invalidated when the element is removed or the
 *       stack is otherwise modified.
 * @ingroup stack_access
 */
void* stack_top(stack_t* stack, error_t* err);

/// @} // stack_access

/// @defgroup stack_modifiers Modifiers
/// @ingroup stack
/// @{

/**
 * @brief Pushes an element onto the top of the stack.
 *
 * @param stack The stack. Must not be NULL.
 * @param item  Pointer to the data to copy in. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @ingroup stack_modifiers
 */
void stack_push(stack_t* stack, const void* item, error_t* err);

/**
 * @brief Removes the element at the top of the stack.
 *
 * @param stack    The stack. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed
 *                 element is copied here before removal. Must be large
 *                 enough to hold `elem_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the stack is empty.
 * @ingroup stack_modifiers
 */
void stack_pop(stack_t* stack, void* item_out, error_t* err);

/// @} // stack_modifiers
/// @} // stack

/// @defgroup map Hash maps
/// @brief Hash map with separate chaining and automatic resizing.
/// @{

/**
 * @brief Signature for a hash function.
 *
 * Must be deterministic: equal keys must always produce the same hash.
 * Should distribute keys uniformly across the output range to minimise
 * bucket collisions.
 *
 * @param key      Pointer to the key data. Must not be NULL.
 * @param key_size Size of the key in bytes.
 * @return Hash value.
 * @ingroup map
 */
typedef size_t (*hash_fn_t)(const void* key, size_t key_size);

/**
 * @brief Signature for a key comparator function.
 *
 * Must be consistent with the hash function: if `cmp(a, b) == 0`
 * then `hash(a) == hash(b)`.
 *
 * @param a        Pointer to the first key. Must not be NULL.
 * @param b        Pointer to the second key. Must not be NULL.
 * @param key_size Size of both keys in bytes.
 * @return 0 if the keys are equal, non-zero otherwise.
 * @ingroup map
 */
typedef int (*cmp_fn_t)(const void* a, const void* b, size_t key_size);

/**
 * @brief Hash map implementing a generic key-value store.
 *
 * Uses separate chaining (one `list_t` per bucket). Keys and values
 * are stored inline in a flat `[key | value]` byte buffer inside each
 * list node, so no per-entry heap allocations are needed beyond the
 * node itself.
 *
 * The table resizes automatically when `count / bucket_count` exceeds
 * `max_load_factor`.
 *
 * @ingroup map
 */
typedef struct {
    list_t** buckets;       ///< Array of per-bucket linked lists.
    size_t   bucket_count;  ///< Number of buckets.
    size_t   count;         ///< Total number of key-value pairs stored.
    size_t   key_size;      ///< Size in bytes of each key.
    size_t   value_size;    ///< Size in bytes of each value.
    float    max_load_factor; ///< Load factor threshold that triggers a resize.
    hash_fn_t hash;         ///< Hash function. @see hash_fn_t
    cmp_fn_t  cmp;          ///< Key comparator. @see cmp_fn_t
} hashmap_t;

/// @defgroup map_allocation Allocation
/// @ingroup map
/// @{

/**
 * @brief Creates a new empty hash map.
 *
 * @param key_size     Size in bytes of each key. Must be > 0.
 * @param value_size   Size in bytes of each value. Must be > 0.
 * @param bucket_count Initial number of buckets. Pass 0 to use the
 *                     default (8 buckets).
 * @param hash         Hash function. Must not be NULL.
 * @param cmp          Key comparator. Must not be NULL. Must be
 *                     consistent with `hash`.
 * @param err          Optional error output. Populated on failure.
 * @return Pointer to the new hash map, or NULL on failure.
 * @ingroup map_allocation
 */
hashmap_t* hashmap_create(
    size_t    key_size,
    size_t    value_size,
    size_t    bucket_count,
    hash_fn_t hash,
    cmp_fn_t  cmp,
    error_t*  err);

/**
 * @brief Destroys the hash map and frees all associated memory.
 *
 * @param map The hash map to destroy. NULL is safe and does nothing.
 * @ingroup map_allocation
 */
void hashmap_destroy(hashmap_t* map);

/// @} // map_allocation

/// @defgroup map_modifiers Modifiers
/// @ingroup map
/// @{

/**
 * @brief Inserts a key-value pair, or updates the value if the key exists.
 *
 * If the load factor exceeds `max_load_factor` after insertion, the
 * table is automatically resized and all entries are rehashed.
 *
 * @param map   The hash map. Must not be NULL.
 * @param key   Pointer to the key data. Must not be NULL.
 * @param value Pointer to the value data. Must not be NULL.
 * @param err   Optional error output. Populated on failure.
 * @ingroup map_modifiers
 */
void hashmap_insert(hashmap_t* map, const void* key, const void* value, error_t* err);

/**
 * @brief Removes the entry with the given key.
 *
 * @param map      The hash map. Must not be NULL.
 * @param key      Key to remove. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the removed value
 *                 is copied here before removal. Must be large enough to
 *                 hold `value_size` bytes.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the key is not found.
 * @ingroup map_modifiers
 */
void hashmap_remove(hashmap_t* map, const void* key, void* item_out, error_t* err);

/// @} // map_modifiers

/// @defgroup map_access Element access
/// @ingroup map
/// @{

/**
 * @brief Retrieves the value associated with a key.
 *
 * @param map      The hash map. Must not be NULL.
 * @param key      Key to look up. Must not be NULL.
 * @param item_out Optional output buffer. If non-NULL, the value is
 *                 copied here. Must be large enough to hold `value_size`
 *                 bytes. If NULL, the call acts as a presence check.
 * @param err      Optional error output. Populated on failure.
 *                 Sets `ERROR_INVALID_ARGS` if the key is not found.
 * @ingroup map_access
 */
void hashmap_get(hashmap_t* map, const void* key, void* item_out, error_t* err);

/// @} // map_access

/// @defgroup hash_functions Built-in hash and comparator functions
/// @brief Ready-to-use hash and comparator pairs for common key types.
///
/// Each hash function has a matching comparator. Always use them together:
/// | Key type          | Hash function  | Comparator   |
/// |-------------------|----------------|--------------|
/// | Fixed-size buffer | `hash_bytes`   | `cmp_bytes`  |
/// | C string (`char[]`)| `hash_string` | `cmp_string` |
/// | `int` / `uint32_t`| `hash_int32`   | `cmp_bytes`  |
/// | `int64_t`         | `hash_int64`   | `cmp_bytes`  |
///
/// @warning Do not use `hash_bytes` with struct keys that have padding
///          bytes — padding is uninitialized and will cause equal structs
///          to hash differently. Zero-initialize the struct with `memset`
///          before use, or define a custom hash over only the meaningful
///          fields.
/// @ingroup map
/// @{

/**
 * @brief FNV-1a hash over a fixed-size byte buffer.
 *
 * Processes all `key_size` bytes. Suitable for integer types, floats,
 * and tightly packed structs without padding.
 *
 * @param key      Pointer to the key. Must not be NULL.
 * @param key_size Number of bytes to hash.
 * @return Hash value.
 * @ingroup hash_functions
 */
size_t hash_bytes(const void* key, size_t key_size);

/**
 * @brief FNV-1a hash over a null-terminated C string.
 *
 * Stops at the null terminator, so only the string content is hashed —
 * trailing bytes in the buffer beyond `\0` are ignored. Pair with
 * `cmp_string`.
 *
 * @param key      Pointer to a null-terminated string. Must not be NULL.
 * @param key_size Upper bound on bytes to scan (typically `sizeof(char[N])`).
 * @return Hash value.
 * @ingroup hash_functions
 */
size_t hash_string(const void* key, size_t key_size);

/**
 * @brief Murmur-inspired hash for 32-bit integer keys.
 *
 * Applies integer mixing for better bucket distribution than a raw
 * FNV-1a pass over 4 bytes, especially for small sequential values.
 * Pair with `cmp_bytes`.
 *
 * @param key      Pointer to an `int` or `uint32_t`. Must not be NULL.
 * @param key_size Not used; present for consistency with `hash_fn_t`.
 * @return Hash value.
 * @ingroup hash_functions
 */
size_t hash_int32(const void* key, size_t key_size);

/**
 * @brief Murmur3 finalizer hash for 64-bit integer keys.
 *
 * Applies a three-round mixing sequence for uniform distribution.
 * Pair with `cmp_bytes`.
 *
 * @param key      Pointer to an `int64_t` or `uint64_t`. Must not be NULL.
 * @param key_size Not used; present for consistency with `hash_fn_t`.
 * @return Hash value.
 * @ingroup hash_functions
 */
size_t hash_int64(const void* key, size_t key_size);

/**
 * @brief Byte-level equality comparator using `memcmp`.
 *
 * Suitable for any fixed-size key where every byte is significant:
 * integers, floats, and padding-free structs. Consistent with
 * `hash_bytes`, `hash_int32`, and `hash_int64`.
 *
 * @param a        Pointer to the first key. Must not be NULL.
 * @param b        Pointer to the second key. Must not be NULL.
 * @param key_size Number of bytes to compare.
 * @return 0 if equal, non-zero otherwise.
 * @ingroup hash_functions
 */
int cmp_bytes(const void* a, const void* b, size_t key_size);

/**
 * @brief String equality comparator using `strncmp`.
 *
 * Compares up to `key_size` bytes, stopping at the null terminator.
 * Consistent with `hash_string`.
 *
 * @param a        Pointer to the first null-terminated string. Must not be NULL.
 * @param b        Pointer to the second null-terminated string. Must not be NULL.
 * @param key_size Maximum number of bytes to compare.
 * @return 0 if equal, non-zero otherwise.
 * @ingroup hash_functions
 */
int cmp_string(const void* a, const void* b, size_t key_size);

/// @} // hash_functions
/// @} // map

/// @defgroup tree Trees
/// @brief N-ary tree with O(1) insertion, removal, and sibling traversal.
/// @{

typedef struct tree_node tree_node_t; ///< Opaque n-ary tree node.

/**
 * @brief Generic n-ary tree.
 *
 * Each node owns its data and may have an arbitrary number of children,
 * linked as a doubly-connected sibling chain. The tree owns all nodes
 * inserted into it and frees them recursively on destroy or erase.
 *
 * @ingroup tree
 */
typedef struct {
    tree_node_t* root;      ///< Root node, or NULL if the tree is empty.
    size_t       count;     ///< Total number of nodes currently in the tree.
    size_t       elem_size; ///< Size in bytes of the data stored in each node.
} tree_t;

/**
 * @brief Controls how a tree walk callback proceeds after visiting a node.
 * @ingroup tree
 */
typedef enum {
    TREE_WALK_CONTINUE,       ///< Continue traversal normally.
    TREE_WALK_SKIP_CHILDREN,  ///< Skip descendants of this node (pre-order only).
    TREE_WALK_STOP,           ///< Abort traversal immediately.
} tree_walk_result_t;

/**
 * @brief Callback invoked for each node during a tree walk.
 *
 * @param node     The current node. Never NULL.
 * @param depth    Depth of the node (root is 0).
 * @param userdata Caller-supplied context pointer.
 * @return A @ref tree_walk_result_t value controlling traversal.
 * @ingroup tree
 */
typedef tree_walk_result_t (*tree_walk_fn)(tree_node_t* node,
                                           size_t depth,
                                           void* userdata);

/// @defgroup tree_allocation Allocation
/// @ingroup tree
/// @{

/**
 * @brief Creates a new empty n-ary tree.
 *
 * The tree owns all nodes inserted into it. Each node stores a deep
 * copy of the caller's data using the specified element size.
 *
 * @param elem_size Size in bytes of the data stored in each node. Must be > 0.
 * @param err       Optional error output. Populated on failure.
 * @return Pointer to the newly created tree, or NULL on failure.
 * @ingroup tree_allocation
 */
tree_t* tree_create(size_t elem_size, error_t* err);

/**
 * @brief Destroys the tree and frees all associated memory.
 *
 * Recursively destroys every node and the data it owns, then frees
 * the tree itself.
 *
 * @param tree The tree to destroy. NULL is safe and does nothing.
 * @ingroup tree_allocation
 */
void tree_destroy(tree_t* tree);

/// @} // tree_allocation

/// @defgroup tree_capacity Capacity
/// @ingroup tree
/// @{

/**
 * @brief Returns true if the tree contains no nodes.
 *
 * @param tree The tree. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return true if `count == 0`, false otherwise or on failure.
 * @ingroup tree_capacity
 */
bool tree_is_empty(tree_t* tree, error_t* err);

/**
 * @brief Returns the total number of nodes in the tree.
 *
 * @param tree The tree. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Node count, or 0 on failure.
 * @ingroup tree_capacity
 */
size_t tree_size(tree_t* tree, error_t* err);

/// @} // tree_capacity

/// @defgroup tree_iteration Iteration
/// @ingroup tree
/// @{

/**
 * @brief Returns the root node of the tree.
 *
 * @param tree The tree. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the root node, or NULL if the tree is empty or on failure.
 * @ingroup tree_iteration
 */
tree_node_t* tree_root(tree_t* tree, error_t* err);

/**
 * @brief Returns the parent of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the parent node, or NULL if `node` is the root.
 *         Reaching the root is not an error; `ERROR_OK` is set.
 * @ingroup tree_iteration
 */
tree_node_t* tree_parent(tree_node_t* node, error_t* err);

/**
 * @brief Returns the first child of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the first child, or NULL if the node has no children or on failure.
 * @ingroup tree_iteration
 */
tree_node_t* tree_first_child(tree_node_t* node, error_t* err);

/**
 * @brief Returns the last child of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the last child, or NULL if the node has no children or on failure.
 * @ingroup tree_iteration
 */
tree_node_t* tree_last_child(tree_node_t* node, error_t* err);

/**
 * @brief Returns the next sibling of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the next sibling, or NULL if `node` is the last sibling.
 *         Reaching the end of the sibling chain is not an error; `ERROR_OK` is set.
 * @ingroup tree_iteration
 */
tree_node_t* tree_next_sibling(tree_node_t* node, error_t* err);

/**
 * @brief Returns the previous sibling of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the previous sibling, or NULL if `node` is the first sibling.
 *         Reaching the start of the sibling chain is not an error; `ERROR_OK` is set.
 * @ingroup tree_iteration
 */
tree_node_t* tree_prev_sibling(tree_node_t* node, error_t* err);

/**
 * @brief Returns true if a node has at least one child.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return true if the node has one or more children, false otherwise or on failure.
 * @ingroup tree_iteration
 */
bool tree_has_children(tree_node_t* node, error_t* err);

/**
 * @brief Returns the number of direct children of a node.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Number of direct children, or 0 on failure.
 *
 * @note Complexity: O(1).
 * @ingroup tree_iteration
 */
size_t tree_child_count(tree_node_t* node, error_t* err);

/**
 * @brief Returns a pointer to the data stored in a node.
 *
 * The returned pointer is valid until the node is removed or the tree
 * is destroyed.
 *
 * @param node The node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the node's data buffer, or NULL on failure.
 * @ingroup tree_iteration
 */
void* tree_node_data(tree_node_t* node, error_t* err);

/// @} // tree_iteration

/// @defgroup tree_modifiers Modifiers
/// @ingroup tree
/// @{

/**
 * @brief Appends a new node as the last child of a parent.
 *
 * If `parent` is NULL and the tree is empty, the new node becomes the
 * root. Passing NULL as `parent` when the tree already has a root, or
 * passing a non-NULL `parent` when the tree is empty, sets
 * `ERROR_INVALID_ARGS`.
 *
 * @param tree   The tree. Must not be NULL.
 * @param parent Parent node, or NULL to create the root.
 * @param item   Data to store in the new node. Must not be NULL.
 * @param err    Optional error output. Populated on failure.
 * @return Pointer to the newly created node, or NULL on failure.
 *
 * @note Complexity: O(1).
 * @ingroup tree_modifiers
 */
tree_node_t* tree_append_child(tree_t* tree, tree_node_t* parent,
                               const void* item, error_t* err);

/**
 * @brief Inserts a new node immediately after the given node in the
 *        sibling chain.
 *
 * The new node shares the same parent as `node`. Passing the root node
 * sets `ERROR_INVALID_ARGS` since the root cannot have siblings.
 *
 * @param tree The tree. Must not be NULL.
 * @param node Node after which the sibling is inserted. Must not be NULL
 *             and must belong to the tree. Must not be the root.
 * @param item Data to store in the new node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the newly created node, or NULL on failure.
 *
 * @note Complexity: O(1).
 * @ingroup tree_modifiers
 */
tree_node_t* tree_append_sibling(tree_t* tree, tree_node_t* node,
                                 const void* item, error_t* err);

/**
 * @brief Inserts a new node immediately before `ref` in the sibling chain.
 *
 * The new node shares the same parent as `ref`. If `ref` is the current
 * first child, the new node becomes the new first child.
 * Passing the root node sets `ERROR_INVALID_ARGS`.
 *
 * @param tree The tree. Must not be NULL.
 * @param ref  Node before which the new node is inserted. Must not be NULL
 *             and must not be the root.
 * @param item Data to store in the new node. Must not be NULL.
 * @param err  Optional error output. Populated on failure.
 * @return Pointer to the newly created node, or NULL on failure.
 *
 * @note Complexity: O(1).
 * @ingroup tree_modifiers
 */
tree_node_t* tree_insert_before(tree_t* tree, tree_node_t* ref,
                                const void* item, error_t* err);

/**
 * @brief Moves an existing node to a new position in the tree.
 *
 * Detaches `node` from its current position and reattaches it under
 * `new_parent`, immediately before `before`. Pass NULL for `before` to
 * append as the last child of `new_parent`.
 *
 * The tree's total node count is unchanged by this operation.
 *
 * @param tree       The tree. Must not be NULL.
 * @param node       Node to move. Must not be NULL. Must not equal `new_parent`.
 * @param new_parent Destination parent. Must not be NULL and must not be
 *                   `node` or a descendant of `node` — no cycle detection
 *                   is performed; violating this is undefined behaviour.
 * @param before     Sibling before which `node` is inserted, or NULL to append.
 *                   If non-NULL, must be a direct child of `new_parent`.
 * @param err        Optional error output. Populated on failure.
 *
 * @note Complexity: O(1).
 * @warning Moving a node to be its own ancestor creates a cycle and is
 *          undefined behaviour. The caller is responsible for preventing this.
 * @ingroup tree_modifiers
 */
void tree_move(tree_t* tree, tree_node_t* node, tree_node_t* new_parent,
               tree_node_t* before, error_t* err);

/**
 * @brief Removes the subtree rooted at the given node.
 *
 * Deletes `node` and all its descendants and updates the tree structure
 * accordingly. If `node` is NULL, the root is erased and the tree
 * becomes empty.
 *
 * @param tree The tree. Must not be NULL.
 * @param node Root of the subtree to remove, or NULL to erase the root.
 * @param err  Optional error output. Populated on failure.
 *
 * @note Complexity: O(d) where d is the size of the subtree being removed.
 * @warning After this call, `node` and all nodes in its subtree are freed
 *          and must not be dereferenced.
 * @ingroup tree_modifiers
 */
void tree_erase(tree_t* tree, tree_node_t* node, error_t* err);

/// @} // tree_modifiers

/// @defgroup tree_traversal Traversal
/// @ingroup tree
/// @{

/**
 * @brief Visits every node in pre-order (parent before children).
 *
 * The callback may return @ref TREE_WALK_SKIP_CHILDREN to skip the
 * descendants of the current node, or @ref TREE_WALK_STOP to abort
 * traversal entirely.
 *
 * @param tree     The tree. Must not be NULL.
 * @param fn       Callback invoked for each node. Must not be NULL.
 * @param userdata Opaque pointer forwarded to every callback invocation.
 * @param err      Optional error output. Populated on failure.
 *
 * @note Complexity: O(n) where n is the total number of nodes.
 * @ingroup tree_traversal
 */
void tree_walk_preorder(tree_t* tree, tree_walk_fn fn,
                        void* userdata, error_t* err);

/**
 * @brief Visits every node in post-order (children before parent).
 *
 * The callback may return @ref TREE_WALK_STOP to abort traversal.
 * @ref TREE_WALK_SKIP_CHILDREN has no effect in post-order since
 * children are visited before the callback fires; it is treated as
 * @ref TREE_WALK_CONTINUE.
 *
 * @param tree     The tree. Must not be NULL.
 * @param fn       Callback invoked for each node. Must not be NULL.
 * @param userdata Opaque pointer forwarded to every callback invocation.
 * @param err      Optional error output. Populated on failure.
 *
 * @note Complexity: O(n) where n is the total number of nodes.
 * @ingroup tree_traversal
 */
void tree_walk_postorder(tree_t* tree, tree_walk_fn fn,
                         void* userdata, error_t* err);

/// @} // tree_traversal
/// @} // tree

#endif // H_CONTAINERS
