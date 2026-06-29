#include <containers.h>
#include <stdio.h>
#include <string.h>

static int failed = 0;
static int total  = 0;

/* ANSI colors */
#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"
#define C_GREEN   "\033[32m"
#define C_RED     "\033[31m"
#define C_CYAN    "\033[36m"
#define C_YELLOW  "\033[33m"

#define ASSERT(cond, msg)                                                  \
    do {                                                                   \
        printf(C_DIM "    • %-80s" C_RESET, (msg));                        \
        if (!(cond)) {                                                     \
            printf(C_RED "✗" C_RESET C_DIM " (%s:%d)\n" C_RESET,           \
                   __FILE__, __LINE__);                                    \
            return 1;                                                      \
        }                                                                  \
        printf(C_GREEN "✓" C_RESET "\n");                                  \
    } while (0)

#define RUN_TEST(name, fun)                                                \
    do {                                                                   \
        printf(C_BOLD C_CYAN "▶ %s\n" C_RESET, (name));                    \
        total++;                                                           \
        if (fun()) {                                                       \
            failed++;                                                      \
            printf("  " C_RED C_BOLD "✗ FAILED" C_RESET "\n\n");           \
        } else {                                                           \
            printf("  " C_GREEN C_BOLD "✓ PASSED" C_RESET "\n\n");         \
        }                                                                  \
    } while (0)

#define SUMMARY()                                                          \
    do {                                                                   \
        printf(C_DIM "───────────────────────────────────────\n" C_RESET); \
        if (failed > 0) {                                                  \
            printf(C_RED C_BOLD "✗ %d/%d tests failed" C_RESET "\n",       \
                   failed, total);                                         \
        } else {                                                           \
            printf(C_GREEN C_BOLD "✓ All %d tests passed" C_RESET "\n",    \
                   total);                                                 \
        }                                                                  \
    } while (0)

/* ---------------------------------------------------------------- */
/* tree_create / tree_destroy                                        */
/* ---------------------------------------------------------------- */

int test_tree_create(void) {
    error_t  err;
    tree_t  *tree;

    tree = tree_create(sizeof(int), &err);
    ASSERT(tree != NULL,              "tree_create returns valid pointer");
    ASSERT(err.code == ERROR_OK,      "no error on valid creation");
    ASSERT(tree->count == 0,          "count starts at zero");
    ASSERT(tree->elem_size == sizeof(int), "elem_size matches requested value");
    ASSERT(tree->root == NULL,        "root is NULL on empty tree");
    tree_destroy(tree);

    tree = tree_create(0, &err);
    ASSERT(tree == NULL,                   "tree_create returns NULL on zero elem_size");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero elem_size");

    tree = tree_create(sizeof(int), NULL);
    ASSERT(tree != NULL, "tree_create works fine with NULL err pointer");
    tree_destroy(tree);

    return 0;
}

int test_tree_destroy(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2;
    int           v = 1;

    ASSERT(tree != NULL, "tree_create succeeds before destroy test");

    /* destroy empty tree */
    tree_destroy(tree);
    ASSERT(1, "tree_destroy does not crash on empty tree");

    /* destroy populated tree: exercises recursive node cleanup */
    tree = tree_create(sizeof(int), &err);
    ASSERT(tree != NULL, "tree_create succeeds for populated destroy test");

    root = tree_append_child(tree, NULL, &v, &err);
    c1   = tree_append_child(tree, root, &v, &err);
    c2   = tree_append_child(tree, root, &v, &err);
    tree_append_child(tree, c1, &v, &err);
    tree_append_child(tree, c1, &v, &err);
    tree_append_child(tree, c2, &v, &err);
    ASSERT(tree->count == 6, "tree has 6 nodes before destroy");

    tree_destroy(tree);
    ASSERT(1, "tree_destroy does not crash on populated tree");

    tree_destroy(NULL);
    ASSERT(1, "tree_destroy is safe to call on NULL");

    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_is_empty / tree_size                                         */
/* ---------------------------------------------------------------- */

int test_tree_capacity(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root;
    int           v = 1;

    ASSERT(tree != NULL, "tree_create succeeds before capacity test");

    ASSERT(tree_is_empty(tree, &err) == true, "newly created tree is empty");
    ASSERT(err.code == ERROR_OK,              "no error on empty tree_is_empty");
    ASSERT(tree_size(tree, &err) == 0,        "size is 0 on empty tree");
    ASSERT(err.code == ERROR_OK,              "no error on empty tree_size");

    root = tree_append_child(tree, NULL, &v, &err);
    ASSERT(tree_is_empty(tree, &err) == false, "tree is not empty after adding root");
    ASSERT(tree_size(tree, &err) == 1,         "size is 1 after adding root");

    tree_append_child(tree, root, &v, &err);
    tree_append_child(tree, root, &v, &err);
    ASSERT(tree_size(tree, &err) == 3, "size is 3 after adding two children");

    /* invalid args */
    err.code = ERROR_OK;
    tree_is_empty(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_is_empty on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_size(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_size on NULL tree sets invalid args error");

    /* NULL err pointer */
    ASSERT(tree_is_empty(tree, NULL) == false, "tree_is_empty works fine with NULL err pointer");
    ASSERT(tree_size(tree, NULL) == 3,         "tree_size works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_append_child                                                 */
/* ---------------------------------------------------------------- */

int test_tree_append_child(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3, *gc1;
    int           a = 10, b = 20, c = 30, d = 40, e = 99;

    ASSERT(tree != NULL, "tree_create succeeds before tree_append_child test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_append_child(NULL, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_child on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_append_child(tree, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_child with NULL item sets invalid args error");

    /* non-NULL parent on empty tree: use a node from a separate tree */
    tree_t *other = tree_create(sizeof(int), &err);
    ASSERT(other != NULL, "second tree created for cross-tree parent test");
    tree_node_t *other_root = tree_append_child(other, NULL, &a, &err);
    ASSERT(other_root != NULL, "other tree root created");

    err.code = ERROR_OK;
    tree_append_child(tree, other_root, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_child with non-NULL parent on empty tree sets invalid args error");
    tree_destroy(other);

    /* create root via NULL parent */
    root = tree_append_child(tree, NULL, &a, &err);
    ASSERT(root != NULL,          "append_child with NULL parent creates root");
    ASSERT(err.code == ERROR_OK,  "no error on root creation");
    ASSERT(tree->count == 1,      "count is 1 after root creation");
    ASSERT(tree->root == root,    "tree->root points to new root");
    ASSERT(*(int*)tree_node_data(root, &err) == 10, "root stores correct value");

    /* second call with NULL parent on non-empty tree */
    err.code = ERROR_OK;
    tree_append_child(tree, NULL, &b, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_child with NULL parent on non-empty tree sets invalid args error");
    ASSERT(tree->count == 1, "count unchanged after failed second root insert");

    /* append children to root */
    c1 = tree_append_child(tree, root, &b, &err);
    ASSERT(c1 != NULL,         "first child of root created successfully");
    ASSERT(err.code == ERROR_OK, "no error on first child creation");
    ASSERT(tree->count == 2,   "count is 2 after first child");

    c2 = tree_append_child(tree, root, &c, &err);
    ASSERT(c2 != NULL,         "second child of root created successfully");
    ASSERT(tree->count == 3,   "count is 3 after second child");

    c3 = tree_append_child(tree, root, &d, &err);
    ASSERT(c3 != NULL,         "third child of root created successfully");
    ASSERT(tree->count == 4,   "count is 4 after third child");

    /* verify first/last child linkage */
    ASSERT(tree_first_child(root, &err) == c1, "first child of root is c1");
    ASSERT(tree_last_child(root, &err)  == c3, "last child of root is c3");

    /* append grandchild */
    gc1 = tree_append_child(tree, c1, &e, &err);
    ASSERT(gc1 != NULL,        "grandchild created successfully");
    ASSERT(tree->count == 5,   "count is 5 after grandchild");
    ASSERT(tree_first_child(c1, &err) == gc1, "gc1 is first child of c1");
    ASSERT(tree_parent(gc1, &err)     == c1,  "gc1 parent is c1");

    /* NULL err pointer */
    tree_append_child(tree, root, &a, NULL);
    ASSERT(1, "append_child works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_append_sibling                                               */
/* ---------------------------------------------------------------- */

int test_tree_append_sibling(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *s1, *s2;
    int           a = 10, b = 20, c = 30, d = 40;

    ASSERT(tree != NULL, "tree_create succeeds before tree_append_sibling test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_append_sibling(NULL, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_sibling on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    root = tree_append_child(tree, NULL, &a, &err);
    tree_append_sibling(tree, NULL, &b, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_sibling with NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_append_sibling(tree, root, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "append_sibling with NULL item sets invalid args error");

    /* set up: root -> [c1] */
    c1 = tree_append_child(tree, root, &b, &err);
    ASSERT(c1 != NULL, "child c1 created before sibling test");

    /* append sibling after c1 -> root -> [c1, s1] */
    s1 = tree_append_sibling(tree, c1, &c, &err);
    ASSERT(s1 != NULL,            "first sibling created successfully");
    ASSERT(err.code == ERROR_OK,  "no error on first sibling creation");
    ASSERT(tree->count == 3,      "count is 3 after first sibling");
    ASSERT(tree_next_sibling(c1, &err) == s1,   "c1->next is s1");
    ASSERT(tree_parent(s1, &err)       == root, "s1 parent is root");
    ASSERT(tree_last_child(root, &err) == s1,   "last child of root updated to s1");

    /* append sibling after s1 -> root -> [c1, s1, s2] */
    s2 = tree_append_sibling(tree, s1, &d, &err);
    ASSERT(s2 != NULL,            "second sibling created successfully");
    ASSERT(tree->count == 4,      "count is 4 after second sibling");
    ASSERT(tree_next_sibling(s1, &err) == s2,   "s1->next is s2");
    ASSERT(tree_last_child(root, &err) == s2,   "last child of root updated to s2");
    ASSERT(tree_next_sibling(s2, &err) == NULL, "s2 has no next sibling");

    /* values are correct */
    ASSERT(*(int*)tree_node_data(c1, &err) == 20, "c1 stores correct value");
    ASSERT(*(int*)tree_node_data(s1, &err) == 30, "s1 stores correct value");
    ASSERT(*(int*)tree_node_data(s2, &err) == 40, "s2 stores correct value");

    /* NULL err pointer */
    tree_append_sibling(tree, c1, &a, NULL);
    ASSERT(1, "append_sibling works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_erase                                                        */
/* ---------------------------------------------------------------- */

int test_tree_erase(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3, *gc1, *gc2;
    int           v = 1;

    ASSERT(tree != NULL, "tree_create succeeds before tree_erase test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_erase(NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_erase on NULL tree sets invalid args error");

    /* NULL node erases the root */
    int           w = 99;
    tree_node_t  *tmp_root = tree_append_child(tree, NULL, &w, &err);
    tree_node_t  *tmp_c    = tree_append_child(tree, tmp_root, &w, &err);
    (void)tmp_c;
    ASSERT(tree->count == 2, "tree has 2 nodes before NULL-node erase test");
    tree_erase(tree, NULL, &err);
    ASSERT(err.code == ERROR_OK,              "tree_erase with NULL node erases root, ERROR_OK");
    ASSERT(tree->count == 0,                  "count is 0 after NULL-node erase");
    ASSERT(tree->root == NULL,                "tree->root is NULL after NULL-node erase");
    ASSERT(tree_is_empty(tree, &err) == true, "tree is empty after NULL-node erase");


    root = tree_append_child(tree, NULL, &v, &err);
    /*
     * build:        root
     *              / | \
     *            c1  c2  c3
     *           /  \
     *         gc1  gc2
     */
    c1  = tree_append_child(tree, root, &v, &err);
    c2  = tree_append_child(tree, root, &v, &err);
    c3  = tree_append_child(tree, root, &v, &err);
    gc1 = tree_append_child(tree, c1,   &v, &err);
    gc2 = tree_append_child(tree, c1,   &v, &err);
    ASSERT(tree->count == 6, "tree has 6 nodes before erase tests");

    /* erase a leaf: gc1 */
    tree_erase(tree, gc1, &err);
    ASSERT(err.code == ERROR_OK, "erase of leaf gc1 succeeds");
    ASSERT(tree->count == 5,     "count is 5 after erasing gc1");
    ASSERT(tree_first_child(c1, &err) == gc2, "gc2 is now first child of c1 after gc1 erased");

    /* erase a subtree: c1 (takes gc2 with it) */
    tree_erase(tree, c1, &err);
    ASSERT(err.code == ERROR_OK,              "erase of subtree c1 succeeds");
    ASSERT(tree->count == 3,                  "count is 3 after erasing c1 subtree");
    ASSERT(tree_first_child(root, &err) == c2, "c2 is now first child of root");
    ASSERT(tree_has_children(c2, &err) == false, "c2 has no children");

    /* erase a mid sibling: c2 — verify prev linkage of c3 is updated */
    tree_erase(tree, c2, &err);
    ASSERT(err.code == ERROR_OK,               "erase of mid sibling c2 succeeds");
    ASSERT(tree->count == 2,                   "count is 2 after erasing c2");
    ASSERT(tree_first_child(root, &err) == c3, "c3 is now only child of root");
    ASSERT(tree_last_child(root,  &err) == c3, "c3 is also last child of root");
    ASSERT(tree_prev_sibling(c3,  &err) == NULL, "c3 prev is NULL after c2 erased");

    /* erase last child: c3 */
    tree_erase(tree, c3, &err);
    ASSERT(err.code == ERROR_OK,                   "erase of last child c3 succeeds");
    ASSERT(tree->count == 1,                       "count is 1 after erasing c3");
    ASSERT(tree_has_children(root, &err) == false, "root has no children after c3 erased");

    /* erase root: tree becomes empty */
    err.code = ERROR_OK;
    tree_erase(tree, root, &err);
    ASSERT(err.code == ERROR_OK,              "erase of root sets ERROR_OK");
    ASSERT(tree->count == 0,                  "count is 0 after erasing root");
    ASSERT(tree->root == NULL,                "tree->root is NULL after erasing root");
    ASSERT(tree_is_empty(tree, &err) == true, "tree reports empty after root erase");

    /* NULL err pointer */
    root = tree_append_child(tree, NULL, &v, NULL);
    tree_erase(tree, root, NULL);
    ASSERT(1, "tree_erase works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_prev_sibling                                                 */
/* ---------------------------------------------------------------- */

int test_tree_prev_sibling(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3;
    int           a = 1, b = 2, c = 3, d = 4;

    ASSERT(tree != NULL, "tree_create succeeds before tree_prev_sibling test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_prev_sibling(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_prev_sibling on NULL node sets invalid args error");

    /*
     * build:  root -> [c1, c2, c3]
     */
    root = tree_append_child(tree, NULL,  &a, &err);
    c1   = tree_append_child(tree, root,  &b, &err);
    c2   = tree_append_child(tree, root,  &c, &err);
    c3   = tree_append_child(tree, root,  &d, &err);

    /* first child has no prev */
    ASSERT(tree_prev_sibling(c1, &err) == NULL, "first child c1 has no prev sibling");
    ASSERT(err.code == ERROR_OK,                "reaching start of siblings is not an error");

    /* mid and last */
    ASSERT(tree_prev_sibling(c2, &err) == c1, "prev sibling of c2 is c1");
    ASSERT(tree_prev_sibling(c3, &err) == c2, "prev sibling of c3 is c2");

    /* root has no prev (no parent) */
    ASSERT(tree_prev_sibling(root, &err) == NULL, "root has no prev sibling");
    ASSERT(err.code == ERROR_OK,                  "no error on root prev sibling");

    /* after erasing c2, c3->prev must be c1 */
    tree_erase(tree, c2, &err);
    ASSERT(tree_prev_sibling(c3, &err) == c1, "c3->prev is c1 after c2 erased");

    /* NULL err pointer */
    ASSERT(tree_prev_sibling(c3, NULL) == c1, "tree_prev_sibling works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_child_count                                                  */
/* ---------------------------------------------------------------- */

int test_tree_child_count(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3;
    int           v = 1;

    ASSERT(tree != NULL, "tree_create succeeds before tree_child_count test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_child_count(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_child_count on NULL node sets invalid args error");

    root = tree_append_child(tree, NULL, &v, &err);
    ASSERT(tree_child_count(root, &err) == 0, "root child_count is 0 before any children");

    c1 = tree_append_child(tree, root, &v, &err);
    ASSERT(tree_child_count(root, &err) == 1, "root child_count is 1 after append_child");

    c2 = tree_append_child(tree, root, &v, &err);
    ASSERT(tree_child_count(root, &err) == 2, "root child_count is 2 after second append_child");

    c3 = tree_append_sibling(tree, c2, &v, &err);
    ASSERT(tree_child_count(root, &err) == 3, "root child_count is 3 after append_sibling");

    /* leaf nodes have count 0 */
    ASSERT(tree_child_count(c1,   &err) == 0, "leaf c1 child_count is 0");
    ASSERT(tree_child_count(c2,   &err) == 0, "leaf c2 child_count is 0");

    /* erase decrements parent's count */
    tree_erase(tree, c2, &err);
    ASSERT(tree_child_count(root, &err) == 2, "root child_count is 2 after erasing c2");

    tree_erase(tree, c1, &err);
    ASSERT(tree_child_count(root, &err) == 1, "root child_count is 1 after erasing c1");

    tree_erase(tree, c3, &err);
    ASSERT(tree_child_count(root, &err) == 0, "root child_count is 0 after erasing last child");

    /* NULL err pointer */
    ASSERT(tree_child_count(root, NULL) == 0, "tree_child_count works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_insert_before                                                */
/* ---------------------------------------------------------------- */

int test_tree_insert_before(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *ins;
    int           a = 10, b = 20, c = 30, d = 99;

    ASSERT(tree != NULL, "tree_create succeeds before tree_insert_before test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_insert_before(NULL, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before on NULL tree sets invalid args error");

    root = tree_append_child(tree, NULL, &a, &err);

    err.code = ERROR_OK;
    tree_insert_before(tree, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before with NULL ref sets invalid args error");

    err.code = ERROR_OK;
    tree_insert_before(tree, root, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before root sets invalid args error");

    err.code = ERROR_OK;
    c1 = tree_append_child(tree, root, &a, &err);
    tree_insert_before(tree, c1, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before with NULL item sets invalid args error");

    /*
     * build:  root -> [c1, c2]
     * insert before c1 -> root -> [ins, c1, c2]
     */
    c2  = tree_append_child(tree, root, &b, &err);
    ins = tree_insert_before(tree, c1, &d, &err);
    ASSERT(ins != NULL,           "insert_before c1 returns valid node");
    ASSERT(err.code == ERROR_OK,  "no error on insert_before c1");
    ASSERT(tree->count == 4,      "count is 4 after insert_before");
    ASSERT(tree_child_count(root, &err) == 3, "root child_count is 3 after insert_before");

    /* ins is new first child */
    ASSERT(tree_first_child(root, &err)   == ins, "ins is now first child of root");
    ASSERT(tree_next_sibling(ins, &err)   == c1,  "ins->next is c1");
    ASSERT(tree_prev_sibling(c1,  &err)   == ins, "c1->prev is ins");
    ASSERT(tree_prev_sibling(ins, &err)   == NULL,"ins->prev is NULL (first child)");
    ASSERT(*(int*)tree_node_data(ins, &err) == 99, "ins stores correct value");

    /* insert before c2 (mid position) -> root -> [ins, c1, mid, c2] */
    int           m = 55;
    tree_node_t  *mid = tree_insert_before(tree, c2, &m, &err);
    ASSERT(mid != NULL,                        "insert_before c2 returns valid node");
    ASSERT(tree->count == 5,                   "count is 5 after second insert_before");
    ASSERT(tree_next_sibling(c1,  &err) == mid, "c1->next is mid");
    ASSERT(tree_next_sibling(mid, &err) == c2,  "mid->next is c2");
    ASSERT(tree_prev_sibling(mid, &err) == c1,  "mid->prev is c1");
    ASSERT(tree_prev_sibling(c2,  &err) == mid, "c2->prev is mid");
    ASSERT(tree_last_child(root,  &err) == c2,  "last child of root is still c2");

    /* NULL err pointer */
    int n = 0;
    tree_node_t *tmp = tree_insert_before(tree, c2, &n, NULL);
    ASSERT(tmp != NULL, "insert_before works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_move                                                         */
/* ---------------------------------------------------------------- */

int test_tree_move(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3, *gc1;
    int           a = 1, b = 2, c = 3, d = 4, e = 5;

    ASSERT(tree != NULL, "tree_create succeeds before tree_move test");

    /* invalid args */
    root = tree_append_child(tree, NULL, &a, &err);
    c1   = tree_append_child(tree, root, &b, &err);

    err.code = ERROR_OK;
    tree_move(NULL, c1, root, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_move on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_move(tree, NULL, root, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_move with NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_move(tree, c1, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_move with NULL new_parent sets invalid args error");

    err.code = ERROR_OK;
    tree_move(tree, root, root, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_move node to itself sets invalid args error");

    err.code = ERROR_OK;
    tree_move(tree, c1, root, root, &err);  /* before=root, not a child of root's new_parent */
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_move with before not child of new_parent sets invalid args error");

    /*
     * build:   root
     *         / | \
     *        c1  c2  c3
     *        |
     *       gc1
     *
     * total: 5 nodes
     */
    c2  = tree_append_child(tree, root, &c, &err);
    c3  = tree_append_child(tree, root, &d, &err);
    gc1 = tree_append_child(tree, c1,   &e, &err);
    ASSERT(tree->count == 5, "tree has 5 nodes before move tests");

    /* move gc1 to root as last child (append) */
    tree_move(tree, gc1, root, NULL, &err);
    ASSERT(err.code == ERROR_OK,  "tree_move gc1 to root (append) succeeds");
    ASSERT(tree->count == 5,      "count unchanged after move");
    ASSERT(tree_child_count(root, &err) == 4,  "root now has 4 children");
    ASSERT(tree_child_count(c1,   &err) == 0,  "c1 now has 0 children");
    ASSERT(tree_last_child(root,  &err) == gc1, "gc1 is last child of root");
    ASSERT(tree_parent(gc1,       &err) == root,"gc1 parent is root");
    ASSERT(tree_prev_sibling(gc1, &err) == c3,  "gc1->prev is c3");
    ASSERT(tree_next_sibling(c3,  &err) == gc1, "c3->next is gc1");

    /* move c3 before c2 -> root -> [c1, c3, c2, gc1] */
    tree_move(tree, c3, root, c2, &err);
    ASSERT(err.code == ERROR_OK,  "tree_move c3 before c2 succeeds");
    ASSERT(tree->count == 5,      "count unchanged after move");
    ASSERT(tree_first_child(root, &err)    == c1,  "first child still c1");
    ASSERT(tree_next_sibling(c1,  &err)    == c3,  "c1->next is c3");
    ASSERT(tree_next_sibling(c3,  &err)    == c2,  "c3->next is c2");
    ASSERT(tree_prev_sibling(c3,  &err)    == c1,  "c3->prev is c1");
    ASSERT(tree_prev_sibling(c2,  &err)    == c3,  "c2->prev is c3");

    /* move c1 to be first child of c2 (append, c2 had no children) */
    tree_move(tree, c1, c2, NULL, &err);
    ASSERT(err.code == ERROR_OK,              "tree_move c1 under c2 succeeds");
    ASSERT(tree->count == 5,                  "count unchanged after move");
    ASSERT(tree_child_count(root, &err) == 3, "root now has 3 children");
    ASSERT(tree_child_count(c2,   &err) == 1, "c2 now has 1 child");
    ASSERT(tree_first_child(root, &err) == c3, "first child of root is now c3");
    ASSERT(tree_first_child(c2,   &err) == c1, "c1 is now child of c2");
    ASSERT(tree_parent(c1,        &err) == c2, "c1 parent is c2");
    ASSERT(tree_prev_sibling(c3,  &err) == NULL, "c3->prev is NULL (new first child)");

    /* NULL err pointer */
    tree_move(tree, c1, root, NULL, NULL);
    ASSERT(1, "tree_move works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_walk_preorder / tree_walk_postorder                          */
/* ---------------------------------------------------------------- */

typedef struct {
    int  visited[16];
    int  depths[16];
    int  count;
} walk_result_t;

static tree_walk_result_t walk_collect(tree_node_t* node, size_t depth, void* userdata) {
    walk_result_t *r = (walk_result_t*)userdata;
    r->visited[r->count] = *(int*)tree_node_data(node, NULL);
    r->depths[r->count]  = (int)depth;
    r->count++;
    return TREE_WALK_CONTINUE;
}

static tree_walk_result_t walk_stop_at_3(tree_node_t* node, size_t depth, void* userdata) {
    walk_result_t *r = (walk_result_t*)userdata;
    r->visited[r->count] = *(int*)tree_node_data(node, NULL);
    r->count++;
    (void)depth;
    return (r->count == 3) ? TREE_WALK_STOP : TREE_WALK_CONTINUE;
}

static tree_walk_result_t walk_skip_c1(tree_node_t* node, size_t depth, void* userdata) {
    walk_result_t *r = (walk_result_t*)userdata;
    int val = *(int*)tree_node_data(node, NULL);
    r->visited[r->count++] = val;
    (void)depth;
    /* skip children of node with value 2 (c1) */
    return (val == 2) ? TREE_WALK_SKIP_CHILDREN : TREE_WALK_CONTINUE;
}

int test_tree_walk(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3;
    int           a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    walk_result_t r;

    ASSERT(tree != NULL, "tree_create succeeds before walk test");

    /* invalid args */
    err.code = ERROR_OK;
    tree_walk_preorder(NULL, walk_collect, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "walk_preorder on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_walk_preorder(tree, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "walk_preorder with NULL callback sets invalid args error");

    err.code = ERROR_OK;
    tree_walk_postorder(NULL, walk_collect, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "walk_postorder on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_walk_postorder(tree, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "walk_postorder with NULL callback sets invalid args error");

    /* walk on empty tree is a no-op */
    memset(&r, 0, sizeof(r));
    tree_walk_preorder(tree, walk_collect, &r, &err);
    ASSERT(err.code == ERROR_OK, "walk_preorder on empty tree sets ERROR_OK");
    ASSERT(r.count == 0,         "walk_preorder on empty tree visits no nodes");

    /*
     * build:        1 (root)
     *              / | \
     *             2   3   4
     *            / \
     *           5   6
     *
     * pre-order:  1, 2, 5, 6, 3, 4
     * post-order: 5, 6, 2, 3, 4, 1
     */
    root = tree_append_child(tree, NULL,  &a, &err);
    c1   = tree_append_child(tree, root,  &b, &err);
    c2   = tree_append_child(tree, root,  &c, &err);
    c3   = tree_append_child(tree, root,  &d, &err);
           tree_append_child(tree, c1,    &e, &err);
           tree_append_child(tree, c1,    &f, &err);
    ASSERT(tree->count == 6, "tree has 6 nodes before walk tests");

    /* pre-order */
    memset(&r, 0, sizeof(r));
    tree_walk_preorder(tree, walk_collect, &r, &err);
    ASSERT(err.code == ERROR_OK, "walk_preorder sets ERROR_OK");
    ASSERT(r.count == 6,         "walk_preorder visits all 6 nodes");
    ASSERT(r.visited[0] == 1,    "pre-order: node 0 is root (1)");
    ASSERT(r.visited[1] == 2,    "pre-order: node 1 is c1 (2)");
    ASSERT(r.visited[2] == 5,    "pre-order: node 2 is gc1 (5)");
    ASSERT(r.visited[3] == 6,    "pre-order: node 3 is gc2 (6)");
    ASSERT(r.visited[4] == 3,    "pre-order: node 4 is c2 (3)");
    ASSERT(r.visited[5] == 4,    "pre-order: node 5 is c3 (4)");
    /* depth checks */
    ASSERT(r.depths[0] == 0,     "root depth is 0");
    ASSERT(r.depths[1] == 1,     "c1 depth is 1");
    ASSERT(r.depths[2] == 2,     "gc1 depth is 2");

    /* post-order */
    memset(&r, 0, sizeof(r));
    tree_walk_postorder(tree, walk_collect, &r, &err);
    ASSERT(err.code == ERROR_OK, "walk_postorder sets ERROR_OK");
    ASSERT(r.count == 6,         "walk_postorder visits all 6 nodes");
    ASSERT(r.visited[0] == 5,    "post-order: node 0 is gc1 (5)");
    ASSERT(r.visited[1] == 6,    "post-order: node 1 is gc2 (6)");
    ASSERT(r.visited[2] == 2,    "post-order: node 2 is c1 (2)");
    ASSERT(r.visited[3] == 3,    "post-order: node 3 is c2 (3)");
    ASSERT(r.visited[4] == 4,    "post-order: node 4 is c3 (4)");
    ASSERT(r.visited[5] == 1,    "post-order: node 5 is root (1)");

    /* TREE_WALK_STOP: abort after 3 nodes */
    memset(&r, 0, sizeof(r));
    tree_walk_preorder(tree, walk_stop_at_3, &r, &err);
    ASSERT(r.count == 3, "walk_preorder STOP aborts after 3 nodes");
    ASSERT(r.visited[0] == 1 && r.visited[1] == 2 && r.visited[2] == 5,
           "walk_preorder STOP visits correct nodes before stopping");

    memset(&r, 0, sizeof(r));
    tree_walk_postorder(tree, walk_stop_at_3, &r, &err);
    ASSERT(r.count == 3, "walk_postorder STOP aborts after 3 nodes");
    ASSERT(r.visited[0] == 5 && r.visited[1] == 6 && r.visited[2] == 2,
           "walk_postorder STOP visits correct nodes before stopping");

    /* TREE_WALK_SKIP_CHILDREN: skip c1's subtree in pre-order
     * expected: 1, 2, 3, 4  (gc1 and gc2 skipped) */
    memset(&r, 0, sizeof(r));
    tree_walk_preorder(tree, walk_skip_c1, &r, &err);
    ASSERT(r.count == 4,      "walk_preorder SKIP_CHILDREN skips c1 subtree");
    ASSERT(r.visited[0] == 1, "skip test: node 0 is root (1)");
    ASSERT(r.visited[1] == 2, "skip test: node 1 is c1 (2)");
    ASSERT(r.visited[2] == 3, "skip test: node 2 is c2 (3)");
    ASSERT(r.visited[3] == 4, "skip test: node 3 is c3 (4)");

    /* NULL err pointer */
    memset(&r, 0, sizeof(r));
    tree_walk_preorder(tree, walk_collect, &r, NULL);
    ASSERT(r.count == 6, "walk_preorder works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

/* ---------------------------------------------------------------- */
/* tree_iteration                                                    */
/* ---------------------------------------------------------------- */

int test_tree_iteration(void) {
    error_t       err;
    tree_t       *tree = tree_create(sizeof(int), &err);
    tree_node_t  *root, *c1, *c2, *c3, *gc1, *gc2;
    int           a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;

    ASSERT(tree != NULL, "tree_create succeeds before iteration test");

    /* tree_root on empty tree */
    ASSERT(tree_root(tree, &err) == NULL, "tree_root on empty tree returns NULL");
    ASSERT(err.code == ERROR_OK,          "no error from tree_root on empty tree");

    /* invalid args */
    err.code = ERROR_OK;
    tree_root(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_root on NULL tree sets invalid args error");

    err.code = ERROR_OK;
    tree_parent(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_parent on NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_first_child(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_first_child on NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_last_child(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_last_child on NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_next_sibling(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_next_sibling on NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_has_children(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_has_children on NULL node sets invalid args error");

    err.code = ERROR_OK;
    tree_node_data(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "tree_node_data on NULL node sets invalid args error");

    /*
     * build:        1 (root)
     *              / | \
     *             2   3   4
     *            / \
     *           5   6
     */
    root = tree_append_child(tree, NULL,  &a, &err);
    c1   = tree_append_child(tree, root,  &b, &err);
    c2   = tree_append_child(tree, root,  &c, &err);
    c3   = tree_append_child(tree, root,  &d, &err);
    gc1  = tree_append_child(tree, c1,    &e, &err);
    gc2  = tree_append_child(tree, c1,    &f, &err);
    ASSERT(tree->count == 6, "tree has 6 nodes before iteration tests");

    /* tree_root */
    ASSERT(tree_root(tree, &err) == root, "tree_root returns correct node");
    ASSERT(err.code == ERROR_OK,          "no error from tree_root");

    /* tree_parent */
    ASSERT(tree_parent(root, &err) == NULL, "tree_parent of root returns NULL");
    ASSERT(err.code == ERROR_OK,            "reaching root via tree_parent is not an error");
    ASSERT(tree_parent(c1,   &err) == root, "tree_parent of c1 is root");
    ASSERT(tree_parent(gc1,  &err) == c1,   "tree_parent of gc1 is c1");

    /* tree_first_child / tree_last_child */
    ASSERT(tree_first_child(root, &err) == c1,  "first child of root is c1");
    ASSERT(tree_last_child(root,  &err) == c3,  "last child of root is c3");
    ASSERT(tree_first_child(c1,   &err) == gc1, "first child of c1 is gc1");
    ASSERT(tree_last_child(c1,    &err) == gc2, "last child of c1 is gc2");
    ASSERT(tree_first_child(c2,   &err) == NULL, "first child of leaf c2 is NULL");
    ASSERT(tree_last_child(c2,    &err) == NULL, "last child of leaf c2 is NULL");

    /* tree_next_sibling */
    ASSERT(tree_next_sibling(c1,  &err) == c2,  "next sibling of c1 is c2");
    ASSERT(tree_next_sibling(c2,  &err) == c3,  "next sibling of c2 is c3");
    ASSERT(tree_next_sibling(c3,  &err) == NULL, "next sibling of last child c3 is NULL");
    ASSERT(err.code == ERROR_OK,                 "reaching end of siblings is not an error");
    ASSERT(tree_next_sibling(gc1, &err) == gc2,  "next sibling of gc1 is gc2");
    ASSERT(tree_next_sibling(gc2, &err) == NULL, "next sibling of last grandchild gc2 is NULL");

    /* tree_has_children */
    ASSERT(tree_has_children(root, &err) == true,  "root has children");
    ASSERT(tree_has_children(c1,   &err) == true,  "c1 has children");
    ASSERT(tree_has_children(c2,   &err) == false, "c2 has no children");
    ASSERT(tree_has_children(gc1,  &err) == false, "gc1 has no children");

    /* tree_node_data */
    ASSERT(*(int*)tree_node_data(root, &err) == 1, "root data is correct");
    ASSERT(*(int*)tree_node_data(c1,   &err) == 2, "c1 data is correct");
    ASSERT(*(int*)tree_node_data(c2,   &err) == 3, "c2 data is correct");
    ASSERT(*(int*)tree_node_data(c3,   &err) == 4, "c3 data is correct");
    ASSERT(*(int*)tree_node_data(gc1,  &err) == 5, "gc1 data is correct");
    ASSERT(*(int*)tree_node_data(gc2,  &err) == 6, "gc2 data is correct");

    /* manual depth-first pre-order traversal: 1,2,5,6,3,4 */
    int expected[] = { 1, 2, 5, 6, 3, 4 };
    int idx = 0;

    /* iterative pre-order DFS: push children right-to-left so
       leftmost is popped and visited first                      */
    tree_node_t *stk[16];
    int          top = 0;
    stk[top++] = root;

    while (top > 0) {
        tree_node_t *n = stk[--top];

        if (*(int*)tree_node_data(n, &err) != expected[idx++]) {
            ASSERT(0, "depth-first pre-order traversal visits nodes in correct order");
        }

        /* collect children into a temporary array so we can push
           them right-to-left                                      */
        tree_node_t *children[16];
        int          nchildren = 0;
        tree_node_t *ch = tree_first_child(n, &err);
        while (ch) {
            children[nchildren++] = ch;
            ch = tree_next_sibling(ch, &err);
        }
        for (int i = nchildren - 1; i >= 0; i--)
            stk[top++] = children[i];
    }

    ASSERT(idx == 6, "depth-first pre-order traversal visits all 6 nodes");
    ASSERT(1,        "depth-first pre-order traversal visits nodes in correct order");

    /* NULL err pointer */
    ASSERT(tree_root(tree, NULL) == root,          "tree_root works fine with NULL err pointer");
    ASSERT(tree_first_child(root, NULL) == c1,     "tree_first_child works fine with NULL err pointer");
    ASSERT(tree_next_sibling(c1, NULL)  == c2,     "tree_next_sibling works fine with NULL err pointer");
    ASSERT(tree_node_data(root, NULL)   != NULL,   "tree_node_data works fine with NULL err pointer");

    tree_destroy(tree);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("tree_create",          test_tree_create);
    RUN_TEST("tree_destroy",         test_tree_destroy);
    RUN_TEST("tree_capacity",        test_tree_capacity);
    RUN_TEST("tree_append_child",    test_tree_append_child);
    RUN_TEST("tree_append_sibling",  test_tree_append_sibling);
    RUN_TEST("tree_erase",           test_tree_erase);
    RUN_TEST("tree_iteration",       test_tree_iteration);
    RUN_TEST("tree_prev_sibling",    test_tree_prev_sibling);
    RUN_TEST("tree_child_count",     test_tree_child_count);
    RUN_TEST("tree_insert_before",   test_tree_insert_before);
    RUN_TEST("tree_move",            test_tree_move);
    RUN_TEST("tree_walk",            test_tree_walk);

    SUMMARY();
    return failed ? 1 : 0;
}
