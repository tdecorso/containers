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

int main(void) {

    RUN_TEST("list_create",      test_list_create);
    RUN_TEST("list_destroy",     test_list_destroy);
    RUN_TEST("list_is_empty",    test_list_is_empty);
    RUN_TEST("list_clear",       test_list_clear);
    RUN_TEST("list_push_back",   test_list_push_back);
    RUN_TEST("list_push_front",  test_list_push_front);
    RUN_TEST("list_pop_back",    test_list_pop_back);
    RUN_TEST("list_pop_front",   test_list_pop_front);

    return 0;
}
