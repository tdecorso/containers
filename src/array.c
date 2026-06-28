#include "containers.h"
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define ARRAY_MAX_SIZE INT_MAX

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
    if (cap * elem_size > ARRAY_MAX_SIZE) {
        error_create(err, ERROR_INVALID_ARGS, "Requested capacity exceeds maximum size.");
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

    if (arr->count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an empty array.");
        return NULL;
    }

    if (index == 0) return array_front(arr, err);

    if (index >= arr->count) {
        error_create(err, ERROR_INVALID_ARGS, "The index is greater than the array count.");
        return NULL;
    }

    error_create(err, ERROR_OK, "No error found.");
    uint8_t* base = arr->data;
    return base + arr->elem_size * index;
}

void* array_front(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return NULL;
    }

    error_create(err, ERROR_OK, "No error found.");

    if (arr->count == 0) {
        return NULL;
    }

    return arr->data;
}

void* array_back(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return NULL;
    }

    error_create(err, ERROR_OK, "No error found.");

    if (arr->count == 0) {
        return NULL;
    }

    uint8_t* base = arr->data;
    return base + (arr->count - 1) * arr->elem_size;
}

bool array_is_empty(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return false;
    }

    error_create(err, ERROR_OK, "No error found.");
    return arr->count == 0;
}

size_t array_max_size(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return 0;
    }

    if (arr->elem_size == 0) {
        error_create(err, ERROR_INVALID_STATE, "The array element size is 0.");
        return 0;
    }

    error_create(err, ERROR_OK, "No error found.");
    return ARRAY_MAX_SIZE / arr->elem_size;
}

void array_reserve(array_t* arr, size_t to_reserve, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }
    if (to_reserve == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Storage to reserve must be greater than zero.");
        return;
    }
    if (arr->elem_size == 0) {
        error_create(err, ERROR_INVALID_STATE, "The array element size is 0.");
        return;
    }
    if (arr->cap == 0) {
        error_create(err, ERROR_INVALID_STATE, "The array capacity is 0.");
        return;
    }

    if (to_reserve * arr->elem_size > ARRAY_MAX_SIZE) {
        error_create(err, ERROR_INVALID_ARGS, "Requested storage exceeds maximum size.");
        return;
    }

    void* data = realloc(arr->data, to_reserve * arr->elem_size);
    if (!data) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Array reallocation failed.");
        return;
    }

    arr->data = data;
    arr->cap = to_reserve;

    error_create(err, ERROR_OK, "No error found.");
}

void array_shrink_to_fit(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }

    if (arr->cap > arr->count) {
        size_t new_cap = arr->count > 0 ? arr->count : 1;
        void* data = realloc(arr->data, new_cap * arr->elem_size);
        if (!data) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Array reallocation failed.");
            return;
        }
        arr->cap = new_cap;
        arr->data = data;
    }

    error_create(err, ERROR_OK, "No error found.");
}

void array_push_back(array_t* arr, const void* data, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }

    if (!data) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item to push back.");
        return;
    }

    if (arr->count == arr->cap) {
        if (arr->cap >= (ARRAY_MAX_SIZE / 2) * arr->elem_size) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Array memory exceeds maximum space.");
            return;
        }
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

void array_clear(array_t* arr, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }

    memset(arr->data, 0, arr->elem_size * arr->cap);
    arr->count = 0;
    error_create(err, ERROR_OK, "No error found.");
}

void array_insert(array_t* arr, size_t index, const void* item, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item to insert.");
        return;
    }
    if (index > arr->count) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an invalid index. Must be <= than count.");
        return;
    }
    if (arr->cap == arr->count) {
        size_t new_cap = arr->cap * 2;
        void* data = realloc(arr->data, new_cap * arr->elem_size);
        if (!data) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Could not allocate memory for the array.");
            return;
        }
        arr->data = data;
        arr->cap = new_cap;
    }
    
    uint8_t* base = arr->data;
    memmove(base + (index + 1) * arr->elem_size,
            base + index * arr->elem_size,
            (arr->count - index) * arr->elem_size);
    memcpy(base + index * arr->elem_size, item, arr->elem_size);
    arr->count++;
    error_create(err, ERROR_OK, "No error found.");
}

void array_erase(array_t* arr, size_t index, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }
    if (index >= arr->count) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an invalid index. Must be < than count.");
        return;
    }

    uint8_t* base = arr->data;
    memmove(base + index * arr->elem_size,
            base + (index + 1) * arr->elem_size,
            (arr->count - index) * arr->elem_size);
    arr->count--;
    error_create(err, ERROR_OK, "No error found.");
}

void array_pop_back(array_t* arr, void* item_out, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }
    if (arr->count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an empty array.");
        return;
    }
    size_t index = arr->count - 1;
    void* src = (char*)arr->data + (index * arr->elem_size);
    if (item_out) {
        memcpy(item_out, src, arr->elem_size);
    }
    arr->count--;
    error_create(err, ERROR_OK, "No error found.");
}

void array_pop_front(array_t* arr, void* item_out, error_t* err) {
    if (!arr) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL array.");
        return;
    }

    if (arr->count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an empty array.");
        return;
    }

    void* first = arr->data;

    if (item_out) {
        memcpy(item_out, first, arr->elem_size);
    }

    size_t bytes_to_move = (arr->count - 1) * arr->elem_size;

    if (bytes_to_move > 0) {
        memmove(first,
                (char*)arr->data + arr->elem_size,
                bytes_to_move);
    }

    arr->count--;
    error_create(err, ERROR_OK, "No error found.");
}
