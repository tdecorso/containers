#include <containers.h>
#include <string.h>

typedef struct tree_node {
    struct tree_node* parent;
    struct tree_node* first_child;
    struct tree_node* last_child;
    struct tree_node* next;
    void* data;
} tree_node_t;

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

static tree_node_t* tree_node_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be 0.");
        return NULL;
    }
    tree_node_t* node = malloc(sizeof(tree_node_t));
    if (!node) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }
    node->parent = NULL;
    node->first_child = NULL;
    node->last_child = NULL;
    node->next = NULL;
    node->data = malloc(elem_size);
    if (!node->data) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        free(node);
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found");
    return node;
}

static void tree_node_destroy(tree_t* tree, tree_node_t* node) {
    if (!node) return;
    if (node->data) free(node->data);
    tree_node_destroy(tree, node->first_child);
    tree_node_destroy(tree, node->next);
    if (tree) tree->count--;
    free(node);
}

tree_t* tree_create(size_t elem_size, error_t* err) {
    if (elem_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Element size cannot be 0.");
        return NULL;
    }
    tree_t* tree = malloc(sizeof(tree_t));
    if (!tree) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    tree->root = NULL;
    tree->count = 0;
    tree->elem_size = elem_size;
    error_create(err, ERROR_OK, "No error found.");
    return tree;
}

void tree_destroy(tree_t* tree) {
    if (!tree) return;
    tree_node_destroy(tree, tree->root);
    free(tree);
}

bool tree_is_empty(tree_t* tree, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return 0;
    }
    error_create(err, ERROR_OK, "No error found.");
    return tree->count == 0;
}

size_t tree_size(tree_t* tree, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return 0;
    }
    error_create(err, ERROR_OK, "No error found.");
    return tree->count;
}

tree_node_t* tree_root(tree_t* tree, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return tree->root;
}

tree_node_t* tree_parent(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->parent;
}

tree_node_t* tree_first_child(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->first_child;
}

tree_node_t* tree_last_child(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->last_child;
}

tree_node_t* tree_next_sibling(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->next;
}

bool tree_has_children(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return 0;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->first_child != NULL;
}

void* tree_node_data(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->data;
}

tree_node_t* tree_append_child(tree_t* tree, tree_node_t* parent, const void* item, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return NULL;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return NULL;
    }

    tree_node_t* node = tree_node_create(tree->elem_size, err);
    if (!node) return NULL;

    memcpy(node->data, item, tree->elem_size);

    if (!parent) {
        if (tree->root) {
            error_create(err, ERROR_INVALID_ARGS, "Tree already has a root node.");
            tree_node_destroy(NULL, node);
            return NULL;
        }
        tree->root = node;
        tree->count++;
        error_create(err, ERROR_OK, "No error found.");
        return node;
    } else if (!tree->root) {
        error_create(err, ERROR_INVALID_ARGS, "You cannot pass a parent node on an empty tree.");
        tree_node_destroy(NULL, node);
        return NULL;
    }

    node->parent = parent;
    if (!parent->first_child) {
        if (parent->last_child) {
            error_create(err, ERROR_INVALID_STATE, "Parent children have corrupted state.");
            tree_node_destroy(NULL, node);
            return NULL;
        }
        parent->first_child = node;
        parent->last_child  = node;
        tree->count++;
        error_create(err, ERROR_OK, "No error found.");
        return node;
    }

    parent->last_child->next = node;
    parent->last_child       = node;
    tree->count++;
    error_create(err, ERROR_OK, "No error found.");
    return node;
}

tree_node_t* tree_append_sibling(tree_t* tree, tree_node_t* node, const void* item, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return NULL;
    }
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return NULL;
    }
    if (!node->parent) {
        error_create(err, ERROR_INVALID_ARGS, "Cannot append a sibling to the root node.");
        return NULL;
    }

    tree_node_t* sibling = tree_node_create(tree->elem_size, err);
    if (err && err->code != ERROR_OK) return NULL;
    if (!sibling) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }
    memcpy(sibling->data, item, tree->elem_size);
    sibling->parent   = node->parent;
    sibling->next     = node->next;
    node->next        = sibling;
    tree->count++;

    if (node->parent->last_child == node)
        node->parent->last_child = sibling;

    error_create(err, ERROR_OK, "No error found.");
    return sibling;
}

void tree_erase(tree_t* tree, tree_node_t* node, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return;
    }

    if (!node) node = tree->root;

    if (!node) {
        error_create(err, ERROR_OK, "No error found.");
        return;
    }

    if (node->parent) {
        tree_node_t *parent = node->parent;

        if (parent->first_child == node) {
            parent->first_child = node->next;
        } else {
            tree_node_t *prev = parent->first_child;
            while (prev && prev->next != node)
                prev = prev->next;
            if (prev) prev->next = node->next;
        }

        if (parent->last_child == node) {
            tree_node_t *cur = parent->first_child;
            tree_node_t *last = NULL;
            while (cur) { last = cur; cur = cur->next; }
            parent->last_child = last;
        }

        node->parent = NULL;
        node->next   = NULL;
    } else {
        tree->root = NULL;
    }

    tree_node_destroy(tree, node);
    error_create(err, ERROR_OK, "No error found.");
}
