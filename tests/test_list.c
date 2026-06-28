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
/* helpers                                                           */
/* ---------------------------------------------------------------- */

static int node_val(list_node_t* node) {
    return *(int*)list_node_data(node, NULL);
}

/* ---------------------------------------------------------------- */
/* list_create / list_destroy                                        */
/* ---------------------------------------------------------------- */

int test_list_create(void) {
    error_t   err;
    list_t   *list;

    list = list_create(sizeof(int), &err);
    ASSERT(list != NULL,             "list_create returns valid pointer");
    ASSERT(err.code == ERROR_OK,     "no error on valid creation");
    ASSERT(list->count == 0,         "count starts at zero");
    ASSERT(list->elem_size == sizeof(int), "elem_size matches requested value");
    ASSERT(list->root == NULL,       "root is NULL on empty list");
    ASSERT(list->tail == NULL,       "tail is NULL on empty list");
    list_destroy(list);

    list = list_create(0, &err);
    ASSERT(list == NULL,                    "list_create returns NULL on zero elem_size");
    ASSERT(err.code == ERROR_INVALID_ARGS,  "invalid args error on zero elem_size");

    list = list_create(sizeof(int), NULL);
    ASSERT(list != NULL, "list_create works fine with NULL err pointer");
    list_destroy(list);

    return 0;
}

int test_list_destroy(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);

    ASSERT(list != NULL, "list_create succeeds before destroy test");

    list_destroy(list);
    ASSERT(1, "list_destroy does not crash on a valid list");

    list_destroy(NULL);
    ASSERT(1, "list_destroy is safe to call on NULL");

    return 0;
}

/* ---------------------------------------------------------------- */
/* list_is_empty                                                     */
/* ---------------------------------------------------------------- */

int test_list_is_empty(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       value = 1;

    ASSERT(list != NULL, "list_create succeeds before list_is_empty test");

    ASSERT(list_is_empty(list, &err) == true,  "newly created list is empty");
    ASSERT(err.code == ERROR_OK,               "no error on empty list check");

    list_push_back(list, &value, &err);
    ASSERT(list_is_empty(list, &err) == false, "list is not empty after push_back");
    ASSERT(err.code == ERROR_OK,               "no error on non-empty list check");

    err.code = ERROR_OK;
    list_is_empty(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_is_empty on NULL list sets invalid args error");

    ASSERT(list_is_empty(list, NULL) == false, "list_is_empty works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_clear                                                        */
/* ---------------------------------------------------------------- */

int test_list_clear(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       value;

    ASSERT(list != NULL, "list_create succeeds before list_clear test");

    /* clear already empty list */
    list_clear(list, &err);
    ASSERT(err.code == ERROR_OK, "list_clear on empty list succeeds silently");
    ASSERT(list->count == 0,     "count remains 0 after clearing empty list");

    /* populate then clear */
    value = 1; list_push_back(list, &value, &err);
    value = 2; list_push_back(list, &value, &err);
    value = 3; list_push_back(list, &value, &err);
    ASSERT(list->count == 3, "list has 3 elements before clear");

    list_clear(list, &err);
    ASSERT(err.code == ERROR_OK,              "list_clear on populated list succeeds");
    ASSERT(list->count == 0,                  "count is 0 after clear");
    ASSERT(list_is_empty(list, &err) == true, "list reports empty after clear");
    ASSERT(list->root == NULL,                "root is NULL after clear");
    ASSERT(list->tail == NULL,                "tail is NULL after clear");

    /* clear twice in a row */
    list_clear(list, &err);
    ASSERT(err.code == ERROR_OK, "list_clear on already cleared list succeeds");

    /* invalid args */
    err.code = ERROR_OK;
    list_clear(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_clear on NULL list sets invalid args error");

    /* NULL err pointer */
    value = 1; list_push_back(list, &value, NULL);
    list_clear(list, NULL);
    ASSERT(1, "list_clear works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_push_back / list_push_front                                  */
/* ---------------------------------------------------------------- */

int test_list_push_back(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       out;

    ASSERT(list != NULL, "list_create succeeds before list_push_back test");

    /* invalid args */
    err.code = ERROR_OK;
    list_push_back(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_back on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_push_back(list, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_back with NULL item sets invalid args error");

    /* single push */
    int a = 10;
    list_push_back(list, &a, &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds on first element");
    ASSERT(list->count == 1,     "count is 1 after first push_back");
    ASSERT(list->root != NULL,   "root is non-NULL after first push_back");
    ASSERT(list->tail != NULL,   "tail is non-NULL after first push_back");
    ASSERT(list->root == list->tail, "root and tail point to same node for single element");

    /* multiple pushes — verify order via pop */
    int b = 20, c = 30;
    list_push_back(list, &b, &err);
    list_push_back(list, &c, &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds on subsequent elements");
    ASSERT(list->count == 3,     "count is 3 after three push_backs");

    list_pop_front(list, &out, &err);
    ASSERT(out == 10, "first popped element is 10 (FIFO order)");
    list_pop_front(list, &out, &err);
    ASSERT(out == 20, "second popped element is 20 (FIFO order)");
    list_pop_front(list, &out, &err);
    ASSERT(out == 30, "third popped element is 30 (FIFO order)");

    /* NULL err pointer */
    list_push_back(list, &a, NULL);
    ASSERT(1, "push_back works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

int test_list_push_front(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       out;

    ASSERT(list != NULL, "list_create succeeds before list_push_front test");

    /* invalid args */
    err.code = ERROR_OK;
    list_push_front(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_front on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_push_front(list, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_front with NULL item sets invalid args error");

    /* single push */
    int a = 10;
    list_push_front(list, &a, &err);
    ASSERT(err.code == ERROR_OK,         "push_front succeeds on first element");
    ASSERT(list->count == 1,             "count is 1 after first push_front");
    ASSERT(list->root == list->tail,     "root and tail point to same node for single element");

    /* multiple pushes — verify reverse order via pop */
    int b = 20, c = 30;
    list_push_front(list, &b, &err);
    list_push_front(list, &c, &err);
    ASSERT(err.code == ERROR_OK, "push_front succeeds on subsequent elements");
    ASSERT(list->count == 3,     "count is 3 after three push_fronts");

    list_pop_front(list, &out, &err);
    ASSERT(out == 30, "first popped element is 30 (LIFO order from front)");
    list_pop_front(list, &out, &err);
    ASSERT(out == 20, "second popped element is 20 (LIFO order from front)");
    list_pop_front(list, &out, &err);
    ASSERT(out == 10, "third popped element is 10 (LIFO order from front)");

    /* NULL err pointer */
    list_push_front(list, &a, NULL);
    ASSERT(1, "push_front works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_pop_back / list_pop_front                                    */
/* ---------------------------------------------------------------- */

int test_list_pop_back(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       out;

    ASSERT(list != NULL, "list_create succeeds before list_pop_back test");

    /* invalid args */
    err.code = ERROR_OK;
    list_pop_back(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_back on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_pop_back(list, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_back on empty list sets invalid args error");

    /* populate: [10, 20, 30] */
    int a = 10, b = 20, c = 30;
    list_push_back(list, &a, &err);
    list_push_back(list, &b, &err);
    list_push_back(list, &c, &err);

    /* pop with item_out */
    list_pop_back(list, &out, &err);
    ASSERT(err.code == ERROR_OK, "pop_back succeeds on populated list");
    ASSERT(out == 30,            "pop_back returns last element");
    ASSERT(list->count == 2,     "count decrements after pop_back");

    list_pop_back(list, &out, &err);
    ASSERT(out == 20, "pop_back returns new last element");
    ASSERT(list->count == 1, "count is 1 after second pop_back");
    ASSERT(list->root == list->tail, "root and tail converge on single remaining element");

    /* pop with NULL item_out */
    list_pop_back(list, NULL, &err);
    ASSERT(err.code == ERROR_OK, "pop_back with NULL item_out succeeds");
    ASSERT(list->count == 0,     "count is 0 after popping last element");
    ASSERT(list->root == NULL,   "root is NULL after list is emptied");
    ASSERT(list->tail == NULL,   "tail is NULL after list is emptied");

    /* pop on newly empty list */
    err.code = ERROR_OK;
    list_pop_back(list, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_back on newly empty list sets invalid args error");

    /* NULL err pointer */
    list_push_back(list, &a, NULL);
    list_pop_back(list, &out, NULL);
    ASSERT(1, "pop_back works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

int test_list_pop_front(void) {
    error_t   err;
    list_t   *list = list_create(sizeof(int), &err);
    int       out;

    ASSERT(list != NULL, "list_create succeeds before list_pop_front test");

    /* invalid args */
    err.code = ERROR_OK;
    list_pop_front(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_front on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_pop_front(list, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_front on empty list sets invalid args error");

    /* populate: [10, 20, 30] */
    int a = 10, b = 20, c = 30;
    list_push_back(list, &a, &err);
    list_push_back(list, &b, &err);
    list_push_back(list, &c, &err);

    /* pop with item_out */
    list_pop_front(list, &out, &err);
    ASSERT(err.code == ERROR_OK, "pop_front succeeds on populated list");
    ASSERT(out == 10,            "pop_front returns first element");
    ASSERT(list->count == 2,     "count decrements after pop_front");

    list_pop_front(list, &out, &err);
    ASSERT(out == 20,            "pop_front returns new first element");
    ASSERT(list->count == 1,     "count is 1 after second pop_front");
    ASSERT(list->root == list->tail, "root and tail converge on single remaining element");

    /* pop with NULL item_out */
    list_pop_front(list, NULL, &err);
    ASSERT(err.code == ERROR_OK, "pop_front with NULL item_out succeeds");
    ASSERT(list->count == 0,     "count is 0 after popping last element");
    ASSERT(list->root == NULL,   "root is NULL after list is emptied");
    ASSERT(list->tail == NULL,   "tail is NULL after list is emptied");

    /* pop on newly empty list */
    err.code = ERROR_OK;
    list_pop_front(list, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "pop_front on newly empty list sets invalid args error");

    /* NULL err pointer */
    list_push_back(list, &a, NULL);
    list_pop_front(list, &out, NULL);
    ASSERT(1, "pop_front works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_insert_before                                                */
/* ---------------------------------------------------------------- */

int test_list_insert_before(void) {
    error_t       err;
    list_t       *list = list_create(sizeof(int), &err);
    list_node_t  *it;
    int           a = 10, b = 20, c = 30, d = 99;

    ASSERT(list != NULL, "list_create succeeds before list_insert_before test");

    /* invalid args */
    err.code = ERROR_OK;
    list_insert_before(NULL, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_insert_before(list, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_before with NULL item sets invalid args error");

    /* NULL pos: insert at end of empty list */
    list_insert_before(list, NULL, &a, &err);
    ASSERT(err.code == ERROR_OK, "insert_before NULL on empty list succeeds");
    ASSERT(list->count == 1,     "count is 1 after inserting into empty list");
    ASSERT(list->root == list->tail, "root and tail are the same for single element");

    /* NULL pos: insert at end of non-empty list -> [10, 20] */
    list_insert_before(list, NULL, &b, &err);
    ASSERT(err.code == ERROR_OK, "insert_before NULL appends to end");
    ASSERT(list->count == 2,     "count is 2 after second insert");
    ASSERT(node_val(list->tail) == 20, "tail holds the last inserted value");

    /* insert before root -> [30, 10, 20] */
    list_insert_before(list, list->root, &c, &err);
    ASSERT(err.code == ERROR_OK,            "insert_before root succeeds");
    ASSERT(list->count == 3,                "count is 3 after inserting before root");
    ASSERT(node_val(list->root) == 30,      "new root has correct value");
    ASSERT(node_val(list->tail) == 20,      "tail unchanged after inserting before root");

    /* insert before tail -> [30, 10, 99, 20] */
    list_insert_before(list, list->tail, &d, &err);
    ASSERT(err.code == ERROR_OK,            "insert_before tail succeeds");
    ASSERT(list->count == 4,                "count is 4 after inserting before tail");
    ASSERT(node_val(list->tail) == 20,      "tail unchanged after inserting before tail");

    /* verify full order via iteration: 30 -> 10 -> 99 -> 20 */
    int expected[] = { 30, 10, 99, 20 };
    int i = 0;
    for (it = list_begin(list, &err); it != list_end(list, &err); it = list_next(it, &err)) {
        if (node_val(it) != expected[i++]) {
            ASSERT(0, "list order is correct after all insert_before operations");
        }
    }
    ASSERT(1, "list order is correct after all insert_before operations");

    /* insert before a mid node -> [30, 10, 77, 99, 20] */
    it = list_next(list_begin(list, &err), &err); /* node holding 10 */
    int mid = 77;
    list_insert_before(list, list_next(it, &err), &mid, &err);
    ASSERT(err.code == ERROR_OK,  "insert_before mid node succeeds");
    ASSERT(list->count == 5,      "count is 5 after mid insert");
    ASSERT(node_val(list_next(it, &err)) == 77, "new node sits immediately after expected predecessor");

    /* NULL err pointer */
    list_insert_before(list, NULL, &a, NULL);
    ASSERT(1, "insert_before works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_insert_after                                                 */
/* ---------------------------------------------------------------- */

int test_list_insert_after(void) {
    error_t       err;
    list_t       *list = list_create(sizeof(int), &err);
    list_node_t  *it;
    int           a = 10, b = 20, c = 30, d = 99;

    ASSERT(list != NULL, "list_create succeeds before list_insert_after test");

    /* invalid args */
    err.code = ERROR_OK;
    list_insert_after(NULL, NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_after on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_insert_after(list, NULL, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert_after with NULL item sets invalid args error");

    /* NULL pos: insert at front of empty list */
    list_insert_after(list, NULL, &a, &err);
    ASSERT(err.code == ERROR_OK,     "insert_after NULL on empty list succeeds");
    ASSERT(list->count == 1,         "count is 1 after inserting into empty list");
    ASSERT(list->root == list->tail, "root and tail are the same for single element");

    /* NULL pos: insert at front of non-empty list -> [20, 10] */
    list_insert_after(list, NULL, &b, &err);
    ASSERT(err.code == ERROR_OK,          "insert_after NULL inserts at front");
    ASSERT(list->count == 2,              "count is 2 after second insert");
    ASSERT(node_val(list->root) == 20,    "root holds the newly prepended value");

    /* insert after root -> [20, 30, 10] */
    list_insert_after(list, list->root, &c, &err);
    ASSERT(err.code == ERROR_OK,          "insert_after root succeeds");
    ASSERT(list->count == 3,              "count is 3 after inserting after root");
    ASSERT(node_val(list->root) == 20,    "root unchanged after inserting after root");
    ASSERT(node_val(list->tail) == 10,    "tail unchanged after inserting after root");
    ASSERT(node_val(list_next(list->root, &err)) == 30, "new node sits immediately after root");

    /* insert after tail -> [20, 30, 10, 99] */
    list_insert_after(list, list->tail, &d, &err);
    ASSERT(err.code == ERROR_OK,       "insert_after tail succeeds");
    ASSERT(list->count == 4,           "count is 4 after inserting after tail");
    ASSERT(node_val(list->tail) == 99, "tail updated to new last node");

    /* verify full order via iteration: 20 -> 30 -> 10 -> 99 */
    int expected[] = { 20, 30, 10, 99 };
    int i = 0;
    for (it = list_begin(list, &err); it != list_end(list, &err); it = list_next(it, &err)) {
        if (node_val(it) != expected[i++]) {
            ASSERT(0, "list order is correct after all insert_after operations");
        }
    }
    ASSERT(1, "list order is correct after all insert_after operations");

    /* NULL err pointer */
    list_insert_after(list, NULL, &a, NULL);
    ASSERT(1, "insert_after works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_erase                                                        */
/* ---------------------------------------------------------------- */

int test_list_erase(void) {
    error_t       err;
    list_t       *list = list_create(sizeof(int), &err);
    list_node_t  *node;
    int           out;
    int           a = 10, b = 20, c = 30;

    ASSERT(list != NULL, "list_create succeeds before list_erase test");

    /* invalid args */
    err.code = ERROR_OK;
    list_erase(NULL, list->root, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_erase on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_erase(list, NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_erase with NULL node sets invalid args error");

    /* populate: [10, 20, 30] */
    list_push_back(list, &a, &err);
    list_push_back(list, &b, &err);
    list_push_back(list, &c, &err);

    /* erase root with item_out -> [20, 30] */
    node = list->root;
    list_erase(list, node, &out, &err);
    ASSERT(err.code == ERROR_OK,       "list_erase on root succeeds");
    ASSERT(out == 10,                  "item_out receives erased root value");
    ASSERT(list->count == 2,           "count decrements after erasing root");
    ASSERT(node_val(list->root) == 20, "new root has correct value");

    /* erase tail with item_out -> [20] */
    node = list->tail;
    list_erase(list, node, &out, &err);
    ASSERT(err.code == ERROR_OK,       "list_erase on tail succeeds");
    ASSERT(out == 30,                  "item_out receives erased tail value");
    ASSERT(list->count == 1,           "count decrements after erasing tail");
    ASSERT(list->root == list->tail,   "root and tail converge on single remaining element");

    /* erase last element with NULL item_out -> [] */
    list_erase(list, list->root, NULL, &err);
    ASSERT(err.code == ERROR_OK,  "list_erase with NULL item_out succeeds");
    ASSERT(list->count == 0,      "count is 0 after erasing last element");
    ASSERT(list->root == NULL,    "root is NULL after list is emptied");
    ASSERT(list->tail == NULL,    "tail is NULL after list is emptied");

    /* erase mid node -> verify neighbours relinked */
    list_push_back(list, &a, &err);
    list_push_back(list, &b, &err);
    list_push_back(list, &c, &err);   /* [10, 20, 30] */

    node = list_next(list->root, &err); /* node holding 20 */
    list_erase(list, node, &out, &err);
    ASSERT(err.code == ERROR_OK,       "list_erase on mid node succeeds");
    ASSERT(out == 20,                  "item_out receives erased mid value");
    ASSERT(list->count == 2,           "count decrements after erasing mid node");
    ASSERT(node_val(list->root) == 10, "root unchanged after mid erase");
    ASSERT(node_val(list->tail) == 30, "tail unchanged after mid erase");
    ASSERT(node_val(list_next(list->root, &err)) == 30, "root->next relinked to tail after mid erase");
    ASSERT(node_val(list_prev(list->tail, &err)) == 10, "tail->prev relinked to root after mid erase");

    /* NULL err pointer */
    list_erase(list, list->root, NULL, NULL);
    ASSERT(1, "list_erase works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

/* ---------------------------------------------------------------- */
/* list_begin / list_end / list_next / list_prev                    */
/* ---------------------------------------------------------------- */

int test_list_iteration(void) {
    error_t       err;
    list_t       *list = list_create(sizeof(int), &err);
    list_node_t  *it;
    int           values[] = { 10, 20, 30, 40, 50 };
    int           i;

    ASSERT(list != NULL, "list_create succeeds before iteration test");

    /* begin / end on empty list */
    it = list_begin(list, &err);
    ASSERT(it == NULL,           "list_begin on empty list returns NULL");
    ASSERT(err.code == ERROR_OK, "no error from list_begin on empty list");

    it = list_end(list, &err);
    ASSERT(it == NULL,           "list_end always returns NULL");
    ASSERT(err.code == ERROR_OK, "no error from list_end");

    /* invalid args */
    err.code = ERROR_OK;
    list_begin(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_begin on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_end(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_end on NULL list sets invalid args error");

    err.code = ERROR_OK;
    list_next(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_next on NULL node sets invalid args error");

    err.code = ERROR_OK;
    list_prev(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "list_prev on NULL node sets invalid args error");

    /* populate: [10, 20, 30, 40, 50] */
    for (i = 0; i < 5; i++)
        list_push_back(list, &values[i], &err);

    /* forward iteration */
    i = 0;
    for (it = list_begin(list, &err); it != list_end(list, &err); it = list_next(it, &err)) {
        if (node_val(it) != values[i++]) {
            ASSERT(0, "forward iteration visits all elements in correct order");
        }
    }
    ASSERT(i == 5, "forward iteration visits exactly 5 elements");
    ASSERT(1,      "forward iteration visits all elements in correct order");

    /* backward iteration */
    i = 4;
    for (it = list->tail; it != NULL; it = list_prev(it, &err)) {
        if (node_val(it) != values[i--]) {
            ASSERT(0, "backward iteration visits all elements in correct order");
        }
    }
    ASSERT(i == -1, "backward iteration visits exactly 5 elements");
    ASSERT(1,       "backward iteration visits all elements in correct order");

    /* list_next on tail returns NULL, ERROR_OK */
    it = list->tail;
    it = list_next(it, &err);
    ASSERT(it == NULL,           "list_next on tail returns NULL");
    ASSERT(err.code == ERROR_OK, "list_next on tail does not set an error");

    /* list_prev on root returns NULL, ERROR_OK */
    it = list->root;
    it = list_prev(it, &err);
    ASSERT(it == NULL,           "list_prev on root returns NULL");
    ASSERT(err.code == ERROR_OK, "list_prev on root does not set an error");

    /* NULL err pointer */
    it = list_begin(list, NULL);
    ASSERT(it != NULL, "list_begin works fine with NULL err pointer");
    list_next(it, NULL);
    ASSERT(1, "list_next works fine with NULL err pointer");
    list_prev(list->tail, NULL);
    ASSERT(1, "list_prev works fine with NULL err pointer");

    list_destroy(list);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("list_create",      test_list_create);
    RUN_TEST("list_destroy",     test_list_destroy);
    RUN_TEST("list_is_empty",    test_list_is_empty);
    RUN_TEST("list_clear",       test_list_clear);
    RUN_TEST("list_push_back",   test_list_push_back);
    RUN_TEST("list_push_front",  test_list_push_front);
    RUN_TEST("list_pop_back",    test_list_pop_back);
    RUN_TEST("list_pop_front",   test_list_pop_front);
    RUN_TEST("list_insert_before", test_list_insert_before);
    RUN_TEST("list_insert_after",  test_list_insert_after);
    RUN_TEST("list_erase",         test_list_erase);
    RUN_TEST("list_iteration",     test_list_iteration);

    return 0;
}
