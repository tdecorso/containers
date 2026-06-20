#include "containers.h"
#include <string.h>
#include <stdint.h>

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

void* array_at(array_t* arr, size_t index, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return NULL;
    }

    if (index == 0) return array_front(arr, err);

    if (index >= arr->count) {
        error_create(err, ERROR_INVALID_ARGS, "The index is greater than the array size.");
        return NULL;
    }

    error_create(err, ERROR_OK, "No error found.");
    uint8_t* base = arr->data;
    return base + arr->elem_size * index;
}

void array_push_back(array_t* arr, void* data, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }

    if (!data) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item to push back.");
        return;
    }

    if (arr->count == arr->cap) {
        size_t new_cap = arr->cap * 2;
        void* data = realloc(arr->data, arr->elem_size * new_cap);
        if (!data) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Could not allocate memory for the array.");
            return;
        }
        arr->data = data;
        arr->cap = new_cap;
    }

    uint8_t* base = arr->data;
    memcpy(base + arr->count * arr->elem_size, data, arr->elem_size);
    arr->count++;
    error_create(err, ERROR_OK, "No error found");
}

void* array_front(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return NULL;
    }

    if (arr->count == 0) {
        error_create(err, ERROR_OK, "No error found.");
        return NULL;
    }

    return arr->data;
}
