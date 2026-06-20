#ifndef H_CONTAINERS
#define H_CONTAINERS

#include <stdlib.h>

typedef enum {
    ERROR_INVALID_ARGS,
    ERROR_OUT_OF_MEMORY,
} error_e;

typedef struct {
    error_e code;
    char msg[256];
} error_t;

/*
 * Generic dynamic array
 */
typedef struct {
    void* data;
    size_t cap;
    size_t count;
    size_t elem_size;
} array_t;

array_t* array_create(size_t elem_size, size_t cap, error_t* err);
void array_destroy(array_t* arr);

#endif // H_CONTAINERS
