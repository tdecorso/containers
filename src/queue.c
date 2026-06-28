#include <containers.h>
#include <string.h>

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

queue_t* queue_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be zero.");
        return NULL;
    }

    queue_t* q = malloc(sizeof(queue_t));
    if (!q) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    q->list = list_create(elem_size, err);
    if (err && err->code != ERROR_OK) {
        free(q);
        return NULL;
    }
    if (q->list == NULL) {
        free(q);
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return q;
}

void queue_destroy(queue_t* queue) {
    if (!queue) return;
    list_destroy(queue->list);
    free(queue);
}

bool queue_is_empty(queue_t* queue, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return 0;
    }

    error_create(err, ERROR_OK, "No error found.");
    return queue->list->count == 0;
}

size_t queue_size(queue_t* queue, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return 0;
    }

    error_create(err, ERROR_OK, "No error found.");
    return queue->list->count;
}

void* queue_front(queue_t* queue, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return NULL;
    }
    if (queue_is_empty(queue, NULL)) {
        error_create(err, ERROR_OK, "No error found.");
        return NULL;
    }

    return list_node_data(queue->list->root, err);
}

void* queue_back(queue_t* queue, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return NULL;
    }
    if (queue_is_empty(queue, NULL)) {
        error_create(err, ERROR_OK, "No error found.");
        return NULL;
    }

    return list_node_data(queue->list->tail, err);
}

void queue_push(queue_t* queue, const void* item, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return;
    }

    return list_push_back(queue->list, item, err);
}

void queue_pop(queue_t* queue, void* item_out, error_t* err) {
    if (!queue) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL queue.");
        return;
    }

    return list_pop_front(queue->list, item_out, err);
}
