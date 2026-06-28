#include <containers.h>
#include <string.h>

#define STACK_DEFAULT_CAPACITY 8

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

stack_t* stack_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be zero.");
        return NULL;
    }
    
    stack_t* s = malloc(sizeof(stack_t));
    if (!s) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    s->arr = array_create(elem_size, STACK_DEFAULT_CAPACITY, err); 
    if (err && err->code != ERROR_OK) {
        free(s);
        return NULL;
    }
    if (!s->arr) {
        free(s);
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found."); 
    return s;
}

void stack_destroy(stack_t* stack) {
    if (!stack) return;
    array_destroy(stack->arr);
    free(stack);
}

bool stack_is_empty(stack_t* stack, error_t* err) {
    if (!stack) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL stack.");
        return 0;
    }
    return array_is_empty(stack->arr, err);
}

size_t stack_size(stack_t* stack, error_t* err) {
    if (!stack) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL stack.");
        return 0;
    }
    error_create(err, ERROR_OK, "No error found.");
    return stack->arr->count;
}

void* stack_top(stack_t* stack, error_t* err) {
    if (!stack) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL stack.");
        return NULL;
    }

    return array_back(stack->arr, err);
}

void stack_push(stack_t* stack, const void* item, error_t* err) {
    if (!stack) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL stack.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return;
    }

    return array_push_back(stack->arr, item, err);
}

void stack_pop(stack_t* stack, void* item_out, error_t* err) {
    if (!stack) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL stack.");
        return;
    }

    return array_pop_back(stack->arr, item_out, err);
}
