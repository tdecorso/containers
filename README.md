# containers.h

A single-header, type-agnostic data structures library in C99.

## Overview

`containers.h` provides the most common container types through a uniform API. Every container operates on `void*` with caller-supplied element sizes, reports errors through an optional `error_t` output parameter, and copies data by value so the caller retains ownership of its memory.

This is primarily an instructional project. The goal is to produce clean, readable, well-documented C code rather than a production-grade, performance-optimised library.

## Containers

| Container | Backing structure | Push | Pop | Access |
|---|---|---|---|---|
| Dynamic array | Contiguous buffer | O(1)† | O(1) | O(1) |
| Doubly-linked list | Heap-allocated nodes | O(1) | O(1) | O(n) |
| Queue | `list_t` | O(1) | O(1) | O(1) |
| Stack | `array_t` | O(1)† | O(1) | O(1) |
| Hash map | Bucket + `list_t` | O(1)† | O(1)† | O(1)† |

† Amortised.

## Usage

### Including the library

```c
#include "containers.h"
```

### Error handling

Every fallible function accepts a trailing `error_t* err` parameter. Passing `NULL` is always safe — errors are silently ignored. On failure the function populates `err->code` with an `error_e` value and `err->msg` with a human-readable description.

```c
error_t err;
array_t* arr = array_create(sizeof(int), 16, &err);
if (!arr) {
    fprintf(stderr, "error %d: %s\n", err.code, err.msg);
    return 1;
}
```

### Dynamic arrays

```c
typedef struct { float x, y; } vec2_t;

array_t* points = array_create(sizeof(vec2_t), 64, NULL);

vec2_t p = { 1.0f, 2.0f };
array_push_back(points, &p, NULL);

vec2_t* q = (vec2_t*)array_front(points, NULL);
printf("%.1f %.1f\n", q->x, q->y);

array_destroy(points);
```

### Linked lists

```c
list_t* list = list_create(sizeof(int), NULL);

int a = 10, b = 20, c = 30;
list_push_back(list, &a, NULL);
list_push_back(list, &b, NULL);
list_push_back(list, &c, NULL);

for (list_node_t* n = list_begin(list, NULL);
     n != list_end(list, NULL);
     n = list_next(n, NULL)) {
    printf("%d\n", *(int*)list_node_data(n, NULL));
}

list_destroy(list);
```

### Queues

```c
queue_t* q = queue_create(sizeof(int), NULL);

int x = 1, y = 2, z = 3;
queue_push(q, &x, NULL);
queue_push(q, &y, NULL);
queue_push(q, &z, NULL);

int out;
while (!queue_is_empty(q, NULL)) {
    queue_pop(q, &out, NULL);
    printf("%d\n", out);   /* prints 1, 2, 3 */
}

queue_destroy(q);
```

### Stacks

```c
stack_t* s = stack_create(sizeof(int), NULL);

int x = 1, y = 2, z = 3;
stack_push(s, &x, NULL);
stack_push(s, &y, NULL);
stack_push(s, &z, NULL);

int out;
while (!stack_is_empty(s, NULL)) {
    stack_pop(s, &out, NULL);
    printf("%d\n", out);   /* prints 3, 2, 1 */
}

stack_destroy(s);
```

### Hash maps

Built-in hash and comparator pairs are provided for the most common key types:

| Key type | Hash function | Comparator |
|---|---|---|
| Fixed-size buffer | `hash_bytes` | `cmp_bytes` |
| C string (`char[]`) | `hash_string` | `cmp_string` |
| `int` / `uint32_t` | `hash_int32` | `cmp_bytes` |
| `int64_t` / `uint64_t` | `hash_int64` | `cmp_bytes` |

```c
hashmap_t* map = hashmap_create(
    sizeof(int), sizeof(float), 0, hash_int32, cmp_bytes, NULL);

int key = 42;
float value = 3.14f;
hashmap_insert(map, &key, &value, NULL);

float out;
hashmap_get(map, &key, &out, NULL);
printf("%.2f\n", out);   /* prints 3.14 */

hashmap_destroy(map);
```

> **Note:** Struct keys with padding bytes must be zero-initialised with
> `memset` before use, or hashing will produce inconsistent results across
> equal structs.

## Building

Requirements: CMake 3.31+, a C99-compatible compiler.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
This produces `libcontainers.a`(or `containers.lib` on Windows) in the build directory. 
To link the library into your own CMake project:
```cmake
target_link_libraries(my_target PRIVATE containers)
```

## Running the tests

After building, run any test binary directly from the build directory.
```bash
./test_array
./test_list
./test_queue
./test_stack
./test_hashmap
```

To check for memory leaks with Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./test_hashmap
```

## Documentation

Documentation is generated with [Doxygen](https://www.doxygen.nl):

```bash
doxygen Doxyfile
```

Then open `docs/html/index.html` in your browser.

## Notes and limitations

- **Not thread-safe.** External synchronisation is required for concurrent access to any container.
- **No iterators for arrays.** Use index-based access via `array_at`, `array_front`, and `array_back`.
- **Early development.** Interfaces may evolve as additional containers are introduced.

## License

Released for educational purposes.
