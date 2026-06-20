#include "containers.h"
#include <string.h>

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

array_t* array_create(size_t elem_size, size_t cap, error_t* err) {
    if (!elem_size || !cap) {
        error_create(err, ERROR_INVALID_ARGS, "Element size or capacity cannot be zero.");
        return NULL;
    }
    array_t* arr = malloc(sizeof(array_t));
    if (!arr) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Could not allocate memory.");
        return NULL;
    }

    arr->data = malloc(elem_size * cap);
    if (!arr->data) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Could not allocate memory.");
        free(arr);
        return NULL;
    }

    arr->count = 0;
    arr->cap = cap;
    arr->elem_size = elem_size;

    error_create(err, ERROR_OK, "No error found.");

    return arr;
}

void array_destroy(array_t* arr) {
    if (!arr) return;
    if (arr->data) free(arr->data);
    free(arr);
}

