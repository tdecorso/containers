#include <containers.h>
#include <string.h>

typedef struct list_node {
    void* data;             
    struct list_node* next; 
    struct list_node* prev;
} list_node_t;

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

static list_node_t* list_node_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be zero.");
        return NULL;
    }
    list_node_t* n = malloc(sizeof(list_node_t));
    if (!n) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }
    n->data = malloc(elem_size);
    if (!n->data) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        free(n);
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    n->next = NULL;
    n->prev = NULL;
    return n;
}

static void list_node_destroy(list_node_t* n) {
    if (!n) return;
    if (n->data) free(n->data);
    free(n);
}

list_t* list_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be zero.");
        return NULL;
    }

    list_t* l = malloc(sizeof(list_t));
    if (!l) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    l->root = NULL;
    l->tail = NULL;
    l->count = 0;
    l->elem_size = elem_size;
    error_create(err, ERROR_OK, "No error found.");
    return l;
}

void list_destroy(list_t* l) {
    if (!l) return;
    list_node_t* iter = l->root;
    while (iter != NULL) {
        list_node_t* next = iter->next;
        list_node_destroy(iter);
        iter = next;
    }
    free(l);
}

bool list_is_empty(list_t* list, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return false;
    }
    error_create(err, ERROR_OK, "No error found.");
    return list->count == 0;
}

void list_clear(list_t* list, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    list_node_t* iter = list->root;
    while (iter != NULL) {
        list_node_t* next = iter->next;
        list_node_destroy(iter);
        iter = next;
    }
    list->root = NULL;
    list->tail = NULL;
    list->count = 0;
}

void list_push_back(list_t* list, const void* item, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return;
    }

    if (!list->root) {
        if (list->tail) {
            error_create(err, ERROR_INVALID_STATE, "Tail node must be NULL if list is empty: corrupted state.");
            return;
        }

        list_node_t* node = list_node_create(list->elem_size, err);
        if (err && err->code != ERROR_OK) return;
        if (!node) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
            return;
        }
        memcpy(node->data, item, list->elem_size);

        list->root = node;
        list->tail = node;
        list->count++;
        error_create(err, ERROR_OK, "No error found.");
        return;
    } 

    list_node_t* node = list_node_create(list->elem_size, err);
    if (err && err->code != ERROR_OK) return;
    if (!node) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
        return;
    }
    memcpy(node->data, item, list->elem_size);

    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->count++;

    error_create(err, ERROR_OK, "No error found.");
}

void list_push_front(list_t* list, const void* item, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return;
    }

    if (!list->root) {
        if (list->tail) {
            error_create(err, ERROR_INVALID_STATE, "Tail node must be NULL if list is empty: corrupted state.");
            return;
        }

        list_node_t* node = list_node_create(list->elem_size, err);
        if (err && err->code != ERROR_OK) return;
        if (!node) {
            error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
            return;
        }
        memcpy(node->data, item, list->elem_size);

        list->root = node;
        list->tail = node;
        list->count++;
        
        error_create(err, ERROR_OK, "No error found.");
        return;
    } 

    list_node_t* node = list_node_create(list->elem_size, err);
    if (err && err->code != ERROR_OK) return;
    if (!node) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
        return;
    }
    memcpy(node->data, item, list->elem_size);

    node->next = list->root;
    list->root->prev = node;
    list->root = node;
    list->count++;

    error_create(err, ERROR_OK, "No error found.");
}

void list_pop_back(list_t* list, void* item_out, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (list->count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an empty list.");
        return;
    }

    list_node_t* node = list->tail->prev;
    if (node) node->next = NULL;
    if (item_out) {
        memcpy(item_out, list->tail->data, list->elem_size);
    }
    list_node_destroy(list->tail);
    list->tail = node;
    if (!list->tail) list->root = NULL;
    list->count--;
    error_create(err, ERROR_OK, "No error found.");
}

void list_pop_front(list_t* list, void* item_out, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (list->count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "You passed an empty list.");
        return;
    }

    list_node_t* node = list->root->next;
    if (node) node->prev = NULL;
    if (item_out) {
        memcpy(item_out, list->root->data, list->elem_size);
    }
    list_node_destroy(list->root);
    list->root = node;
    if (!list->root) list->tail = NULL;
    list->count--;
    error_create(err, ERROR_OK, "No error found.");
}

list_node_t* list_begin(list_t* list, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return list->root;
}

list_node_t* list_end(list_t* list, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return list->tail ? list->tail->next : NULL;
}

list_node_t* list_next(list_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->next;
}

list_node_t* list_prev(list_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->prev;
}

void list_insert_before(list_t* list, list_node_t* pos, const void* item, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return;
    }
    if (!pos) return list_push_back(list, item, err);

    list_node_t* node = list_node_create(list->elem_size, err);
    if (err && err->code != ERROR_OK) return;
    if (!node) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
        return;
    }
    memcpy(node->data, item, list->elem_size);

    node->next = pos;
    node->prev = pos->prev;
    if (node->prev) node->prev->next = node;
    else list->root = node;
    pos->prev = node;

    list->count++;
    error_create(err, ERROR_OK, "No error found.");
}

void list_insert_after(list_t* list, list_node_t* pos, const void* item, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return;
    }

    if (!pos) return list_push_front(list, item, err);

    list_node_t* node = list_node_create(list->elem_size, err);
    if (err && err->code != ERROR_OK) return;
    if (!node) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Node creation failed.");
        return;
    }
    memcpy(node->data, item, list->elem_size);

    node->prev = pos;
    node->next = pos->next;
    if (node->next) node->next->prev = node;
    else list->tail = node;
    pos->next = node;

    list->count++;
    error_create(err, ERROR_OK, "No error found.");
}

void list_erase(list_t* list, list_node_t* node, void* item_out, error_t* err) {
    if (!list) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL list.");
        return;
    }
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return;
    }
    if (node->prev) node->prev->next = node->next;
    else list->root = node->next;
    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;
    if (item_out) {
        memcpy(item_out, node->data, list->elem_size);
    }
    list_node_destroy(node);
    list->count--;
    error_create(err, ERROR_OK, "No error found.");
}

void* list_node_data(list_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }

    error_create(err, ERROR_OK, "No error found.");
    return node->data;
}
