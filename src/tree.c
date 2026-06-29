#include <containers.h>
#include <string.h>

typedef struct tree_node {
    struct tree_node* parent;
    struct tree_node* first_child;
    struct tree_node* last_child;
    struct tree_node* next;
    struct tree_node* prev;
    size_t            child_count;
    void*             data;
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
    node->parent      = NULL;
    node->first_child = NULL;
    node->last_child  = NULL;
    node->next        = NULL;
    node->prev        = NULL;
    node->child_count = 0;
    node->data        = malloc(elem_size);
    if (!node->data) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        free(node);
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node;
}

/*
 * Recursively destroys a node and all its descendants.
 * Recurses only through first_child — siblings are the caller's
 * responsibility. tree_erase nulls next before calling this; tree_destroy
 * relies on the same guarantee via the recursive first_child walk.
 */
static void tree_node_destroy(tree_t* tree, tree_node_t* node) {
    if (!node) return;
    tree_node_t* child = node->first_child;
    while (child) {
        tree_node_t* next_child = child->next;
        tree_node_destroy(tree, child);
        child = next_child;
    }
    if (node->data) free(node->data);
    if (tree) tree->count--;
    free(node);
}

/* Unlinks node from its parent's sibling chain and child_count.
 * Does not free anything. Safe to call on the root (no-op for parent). */
static void tree_node_unlink(tree_node_t* node) {
    tree_node_t* parent = node->parent;
    if (!parent) return;

    if (node->prev)
        node->prev->next = node->next;
    else
        parent->first_child = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        parent->last_child = node->prev;

    parent->child_count--;
    node->parent = NULL;
    node->prev   = NULL;
    node->next   = NULL;
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
    tree->root      = NULL;
    tree->count     = 0;
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
        return false;
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

tree_node_t* tree_prev_sibling(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->prev;
}

bool tree_has_children(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return false;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->first_child != NULL;
}

size_t tree_child_count(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return 0;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->child_count;
}

void* tree_node_data(tree_node_t* node, error_t* err) {
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return NULL;
    }
    error_create(err, ERROR_OK, "No error found.");
    return node->data;
}

tree_node_t* tree_append_child(tree_t* tree, tree_node_t* parent,
                               const void* item, error_t* err) {
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
    }

    if (!tree->root) {
        error_create(err, ERROR_INVALID_ARGS,
                     "You cannot pass a parent node on an empty tree.");
        tree_node_destroy(NULL, node);
        return NULL;
    }

    if (!parent->first_child != !parent->last_child) {
        error_create(err, ERROR_INVALID_STATE,
                     "Parent children have corrupted state.");
        tree_node_destroy(NULL, node);
        return NULL;
    }

    node->parent = parent;
    node->prev   = parent->last_child;

    if (parent->last_child)
        parent->last_child->next = node;
    else
        parent->first_child = node;

    parent->last_child = node;
    parent->child_count++;
    tree->count++;
    error_create(err, ERROR_OK, "No error found.");
    return node;
}

tree_node_t* tree_append_sibling(tree_t* tree, tree_node_t* node,
                                 const void* item, error_t* err) {
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
        error_create(err, ERROR_INVALID_ARGS,
                     "Cannot append a sibling to the root node.");
        return NULL;
    }

    tree_node_t* sibling = tree_node_create(tree->elem_size, err);
    if (!sibling) return NULL;

    memcpy(sibling->data, item, tree->elem_size);

    sibling->parent = node->parent;
    sibling->prev   = node;
    sibling->next   = node->next;

    if (node->next)
        node->next->prev = sibling;
    else
        node->parent->last_child = sibling;

    node->next = sibling;
    node->parent->child_count++;
    tree->count++;
    error_create(err, ERROR_OK, "No error found.");
    return sibling;
}

/*
 * Inserts a new node immediately before `ref` in the sibling chain.
 * `ref` must not be the root. `ref` must not be NULL.
 * If `ref` is the first child, the new node becomes the new first child.
 */
tree_node_t* tree_insert_before(tree_t* tree, tree_node_t* ref,
                                const void* item, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return NULL;
    }
    if (!ref) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL ref node.");
        return NULL;
    }
    if (!item) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL item.");
        return NULL;
    }
    if (!ref->parent) {
        error_create(err, ERROR_INVALID_ARGS,
                     "Cannot insert a sibling before the root node.");
        return NULL;
    }

    tree_node_t* node = tree_node_create(tree->elem_size, err);
    if (!node) return NULL;

    memcpy(node->data, item, tree->elem_size);

    node->parent = ref->parent;
    node->next   = ref;
    node->prev   = ref->prev;

    if (ref->prev)
        ref->prev->next = node;
    else
        ref->parent->first_child = node;

    ref->prev = node;
    ref->parent->child_count++;
    tree->count++;
    error_create(err, ERROR_OK, "No error found.");
    return node;
}

/*
 * Moves `node` to a new position in the tree.
 *
 * `new_parent` must not be NULL and must not be `node` or a descendant
 * of `node` (caller's responsibility — no cycle detection is performed).
 *
 * `before` is the sibling that the moved node will precede. Pass NULL
 * to append as the last child of `new_parent`.
 */
void tree_move(tree_t* tree, tree_node_t* node, tree_node_t* new_parent,
               tree_node_t* before, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return;
    }
    if (!node) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL node.");
        return;
    }
    if (!new_parent) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL new_parent.");
        return;
    }
    if (node == new_parent) {
        error_create(err, ERROR_INVALID_ARGS,
                     "Cannot move a node to be its own parent.");
        return;
    }
    if (before && before->parent != new_parent) {
        error_create(err, ERROR_INVALID_ARGS,
                     "before node does not belong to new_parent.");
        return;
    }

    if (node->parent) {
        tree_node_unlink(node);
    } else {
        tree->root = NULL;
    }

    node->parent = new_parent;

    if (before) {
        node->prev = before->prev;
        node->next = before;

        if (before->prev)
            before->prev->next = node;
        else
            new_parent->first_child = node;

        before->prev = node;
    } else {
        node->prev = new_parent->last_child;
        node->next = NULL;

        if (new_parent->last_child)
            new_parent->last_child->next = node;
        else
            new_parent->first_child = node;

        new_parent->last_child = node;
    }

    new_parent->child_count++;
    error_create(err, ERROR_OK, "No error found.");
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
        tree_node_unlink(node);
    } else {
        tree->root = NULL;
    }

    tree_node_destroy(tree, node);
    error_create(err, ERROR_OK, "No error found.");
}

/* ── Traversal ─────────────────────────────────────────────────────────────── */

/*
 * Internal recursive pre-order walk.
 * Returns false if the callback requested TREE_WALK_STOP.
 */
static bool walk_preorder(tree_node_t* node, size_t depth,
                          tree_walk_fn fn, void* userdata) {
    if (!node) return true;

    tree_walk_result_t result = fn(node, depth, userdata);
    if (result == TREE_WALK_STOP)          return false;
    if (result == TREE_WALK_SKIP_CHILDREN) return true;

    tree_node_t* child = node->first_child;
    while (child) {
        tree_node_t* next = child->next;   /* safe if fn mutates siblings */
        if (!walk_preorder(child, depth + 1, fn, userdata)) return false;
        child = next;
    }
    return true;
}

/*
 * Internal recursive post-order walk.
 * Returns false if the callback requested TREE_WALK_STOP.
 * TREE_WALK_SKIP_CHILDREN has no effect in post-order (children are
 * visited before the callback fires), so it is treated as CONTINUE.
 */
static bool walk_postorder(tree_node_t* node, size_t depth,
                           tree_walk_fn fn, void* userdata) {
    if (!node) return true;

    tree_node_t* child = node->first_child;
    while (child) {
        tree_node_t* next = child->next;
        if (!walk_postorder(child, depth + 1, fn, userdata)) return false;
        child = next;
    }

    tree_walk_result_t result = fn(node, depth, userdata);
    return result != TREE_WALK_STOP;
}

void tree_walk_preorder(tree_t* tree, tree_walk_fn fn,
                        void* userdata, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return;
    }
    if (!fn) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL callback.");
        return;
    }
    error_create(err, ERROR_OK, "No error found.");
    walk_preorder(tree->root, 0, fn, userdata);
}

void tree_walk_postorder(tree_t* tree, tree_walk_fn fn,
                         void* userdata, error_t* err) {
    if (!tree) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL tree.");
        return;
    }
    if (!fn) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL callback.");
        return;
    }
    error_create(err, ERROR_OK, "No error found.");
    walk_postorder(tree->root, 0, fn, userdata);
}
