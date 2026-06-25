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

int test_array_create(void) {
    error_t   err;
    array_t  *arr;

    arr = array_create(sizeof(int), 10, &err);
    ASSERT(arr != NULL,          "array_create returns valid pointer");
    ASSERT(err.code == ERROR_OK, "no error on valid creation");
    ASSERT(arr->cap == 10,       "capacity matches requested value");
    ASSERT(arr->count == 0,      "count starts at zero");
    ASSERT(arr->elem_size == sizeof(int), "elem_size matches requested value");
    array_destroy(arr);

    arr = array_create(sizeof(int), 0, &err);
    ASSERT(arr == NULL,                    "array_create returns NULL on zero capacity");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero capacity");

    arr = array_create(0, 10, &err);
    ASSERT(arr == NULL,                    "array_create returns NULL on zero elem_size");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero elem_size");

    arr = array_create(sizeof(int), 5, NULL);
    ASSERT(arr != NULL, "array_create works fine with NULL err pointer");
    array_destroy(arr);

    return 0;
}

int test_array_destroy(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);

    ASSERT(arr != NULL, "array_create succeeds before destroy test");

    array_destroy(arr);
    ASSERT(1, "array_destroy does not crash on a valid array");

    array_destroy(NULL);
    ASSERT(1, "array_destroy is safe to call on NULL");

    return 0;
}

int test_array_at(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);
    int       value = 42;
    void     *p;

    ASSERT(arr != NULL, "array_create succeeds before array_at test");

    p = array_at(arr, 0, &err);
    ASSERT(p == NULL, "array_at on empty array with index 0 returns NULL");

    array_push_back(arr, &value, &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds before access");

    p = array_at(arr, 0, &err);
    ASSERT(p != NULL,                  "array_at returns valid pointer for in-bounds index");
    ASSERT(*(int *)p == 42,            "array_at returns correct value");
    ASSERT(err.code == ERROR_OK,       "no error on valid access");

    err.code = ERROR_OK;
    p = array_at(arr, 99, &err);
    ASSERT(p == NULL,            "array_at returns NULL on out-of-bounds index");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args err on out-of-bounds access");

    p = array_at(arr, 0, NULL);
    ASSERT(p != NULL, "array_at works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int test_array_front(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);
    int       a = 7, b = 8;
    void     *p;

    ASSERT(arr != NULL, "array_create succeeds before array_front test");

    p = array_front(arr, &err);
    ASSERT(p == NULL,            "array_front on empty array returns NULL");
    ASSERT(err.code == ERROR_OK, "empty array is not treated as an error");

    array_push_back(arr, &a, &err);
    array_push_back(arr, &b, &err);

    p = array_front(arr, &err);
    ASSERT(p != NULL,       "array_front returns valid pointer after pushes");
    ASSERT(*(int *)p == 7,  "array_front returns the first pushed element");

    p = array_front(arr, NULL);
    ASSERT(p != NULL, "array_front works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int test_array_back(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);
    int       a = 7, b = 8;
    void     *p;

    ASSERT(arr != NULL, "array_create succeeds before array_back test");

    p = array_front(arr, &err);
    ASSERT(p == NULL,            "array_back on empty array returns NULL");
    ASSERT(err.code == ERROR_OK, "empty array is not treated as an error");

    array_push_back(arr, &a, &err);
    array_push_back(arr, &b, &err);

    p = array_back(arr, &err);
    ASSERT(p != NULL,       "array_back returns valid pointer after pushes");
    ASSERT(*(int *)p == 8,  "array_back returns the last pushed element");

    p = array_back(arr, NULL);
    ASSERT(p != NULL, "array_back works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int test_array_capacity(void) {
    error_t  err;
    array_t* arr = array_create(sizeof(int), 100, &err);

    ASSERT(array_is_empty(arr, NULL), "array_is_empty returns true on empty array");

    size_t max_size = array_max_size(arr, NULL);

    array_reserve(arr, max_size, &err);

    ASSERT(err.code == ERROR_OK, "array_reserve accepts maximum size as input");

    array_shrink_to_fit(arr, NULL);

    ASSERT(arr->cap == 1, "array_shrink_to_fit allocates storage for 1 element if array is empty");

    array_destroy(arr);
    return 0;
}

int test_array_push_back(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 2, &err);
    int       values[] = { 1, 2, 3, 4, 5 };
    size_t    i;

    ASSERT(arr != NULL, "array_create succeeds before push_back test");

    array_push_back(arr, &values[0], &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds on first element");
    ASSERT(arr->count == 1,      "count increments after push_back");

    array_push_back(arr, &values[1], &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds up to capacity");
    ASSERT(arr->count == 2,      "count matches initial capacity");

    /* triggers growth: count(2) == cap(2) */
    array_push_back(arr, &values[2], &err);
    ASSERT(err.code == ERROR_OK, "push_back succeeds past initial capacity");
    ASSERT(arr->count == 3,      "count increments after growth");
    ASSERT(arr->cap >= 3,        "capacity grows to accommodate new element");

    array_push_back(arr, &values[3], &err);
    array_push_back(arr, &values[4], &err);
    ASSERT(arr->count == 5, "count tracks multiple pushes past growth");

    for (i = 0; i < 5; i++) {
        void *p = array_at(arr, i, &err);
        if (p == NULL || *(int *)p != values[i]) {
            ASSERT(0, "all pushed elements retain correct values and order");
        }
    }
    ASSERT(1, "all pushed elements retain correct values and order");

    array_push_back(arr, &values[0], NULL);
    ASSERT(1, "push_back works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int test_array_insert(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);
    int       value;
    void     *p;

    ASSERT(arr != NULL, "array_create succeeds before array_insert test");

    /* ---- invalid args ---- */

    err.code = ERROR_OK;
    array_insert(NULL, 0, &value, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert on NULL array sets invalid args error");

    err.code = ERROR_OK;
    array_insert(arr, 0, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert with NULL item sets invalid args error");

    /* ---- insert into populated region: shifts right ---- */

    value = 10;
    array_insert(arr, 0, &value, &err);
    ASSERT(err.code == ERROR_OK, "insert at index 0 into empty array succeeds");
    ASSERT(arr->count == 1,      "count is 1 after first insert");

    p = array_at(arr, 0, &err);
    ASSERT(p != NULL,       "element at index 0 is accessible");
    ASSERT(*(int *)p == 10, "element at index 0 has correct value");

    value = 20;
    array_insert(arr, 0, &value, &err);
    ASSERT(err.code == ERROR_OK, "insert at index 0 shifts existing elements right");
    ASSERT(arr->count == 2,      "count is 2 after second insert");

    p = array_at(arr, 0, &err);
    ASSERT(*(int *)p == 20, "newly inserted element sits at index 0");
    p = array_at(arr, 1, &err);
    ASSERT(*(int *)p == 10, "original element shifted to index 1");

    value = 15;
    array_insert(arr, 1, &value, &err);
    ASSERT(err.code == ERROR_OK, "insert at index 1 shifts tail right");
    ASSERT(arr->count == 3,      "count is 3 after mid insert");

    p = array_at(arr, 0, &err);
    ASSERT(*(int *)p == 20, "element at index 0 unchanged after mid insert");
    p = array_at(arr, 1, &err);
    ASSERT(*(int *)p == 15, "newly inserted element sits at index 1");
    p = array_at(arr, 2, &err);
    ASSERT(*(int *)p == 10, "tail element shifted to index 2");

    /* ---- shift insert triggers realloc when count == cap ---- */

    array_destroy(arr);
    arr = array_create(sizeof(int), 2, &err);
    ASSERT(arr != NULL, "array_create succeeds for count==cap shift test");

    value = 1; array_insert(arr, 0, &value, &err);
    value = 2; array_insert(arr, 1, &value, &err);
    ASSERT(arr->count == 2, "array is full before shift-growth test");
    ASSERT(arr->cap   == 2, "capacity is exactly 2 before shift-growth test");

    value = 9;
    array_insert(arr, 0, &value, &err);   /* must realloc before shifting */
    ASSERT(err.code == ERROR_OK, "insert at index 0 when count==cap succeeds");
    ASSERT(arr->cap >= 3,        "capacity grew to accommodate shift");
    ASSERT(arr->count == 3,      "count is 3 after shift-growth insert");

    p = array_at(arr, 0, &err);
    ASSERT(*(int *)p == 9, "inserted element is at index 0 after shift-growth");
    p = array_at(arr, 1, &err);
    ASSERT(*(int *)p == 1, "first original element shifted to index 1");
    p = array_at(arr, 2, &err);
    ASSERT(*(int *)p == 2, "second original element shifted to index 2");

    /* ---- NULL err pointer is safe ---- */

    value = 77;
    array_insert(arr, 0, &value, NULL);
    ASSERT(1, "insert works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int test_array_push_back_invalid_args(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 4, &err);
    int       value = 1;

    ASSERT(arr != NULL, "array_create succeeds before invalid-args test");

    array_push_back(NULL, &value, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_back on NULL array sets invalid args error");

    err.code = ERROR_OK;
    array_push_back(arr, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "push_back with NULL data sets invalid args error");

    array_destroy(arr);
    return 0;
}

int test_array_erase(void) {
    error_t   err;
    array_t  *arr = array_create(sizeof(int), 8, &err);
    int       value;
    void     *p;

    ASSERT(arr != NULL, "array_create succeeds before array_erase test");

    /* ---- invalid args ---- */

    err.code = ERROR_OK;
    array_erase(NULL, 0, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "erase on NULL array sets invalid args error");

    err.code = ERROR_OK;
    array_erase(arr, 0, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "erase on empty array sets invalid args error");

    /* populate: [10, 20, 30, 40, 50] */
    value = 10; array_insert(arr, 0, &value, &err);
    value = 20; array_insert(arr, 1, &value, &err);
    value = 30; array_insert(arr, 2, &value, &err);
    value = 40; array_insert(arr, 3, &value, &err);
    value = 50; array_insert(arr, 4, &value, &err);
    ASSERT(arr->count == 5, "array has 5 elements before erase tests");

    err.code = ERROR_OK;
    array_erase(arr, 99, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "erase at index >= count sets invalid args error");
    ASSERT(arr->count == 5, "count unchanged after out-of-bounds erase");

    /* ---- erase from the middle ---- */

    /* [10, 20, 30, 40, 50] -> erase index 2 -> [10, 20, 40, 50] */
    array_erase(arr, 2, &err);
    ASSERT(err.code == ERROR_OK, "erase at middle index succeeds");
    ASSERT(arr->count == 4,      "count decrements after middle erase");

    p = array_at(arr, 0, &err); ASSERT(*(int *)p == 10, "index 0 unchanged after middle erase");
    p = array_at(arr, 1, &err); ASSERT(*(int *)p == 20, "index 1 unchanged after middle erase");
    p = array_at(arr, 2, &err); ASSERT(*(int *)p == 40, "index 2 shifted left after middle erase");
    p = array_at(arr, 3, &err); ASSERT(*(int *)p == 50, "index 3 shifted left after middle erase");

    /* ---- erase from the head ---- */

    /* [10, 20, 40, 50] -> erase index 0 -> [20, 40, 50] */
    array_erase(arr, 0, &err);
    ASSERT(err.code == ERROR_OK, "erase at index 0 succeeds");
    ASSERT(arr->count == 3,      "count decrements after head erase");

    p = array_at(arr, 0, &err); ASSERT(*(int *)p == 20, "index 0 is old index 1 after head erase");
    p = array_at(arr, 1, &err); ASSERT(*(int *)p == 40, "index 1 is old index 2 after head erase");
    p = array_at(arr, 2, &err); ASSERT(*(int *)p == 50, "index 2 is old index 3 after head erase");

    /* ---- erase from the tail ---- */

    /* [20, 40, 50] -> erase index 2 -> [20, 40] */
    array_erase(arr, 2, &err);
    ASSERT(err.code == ERROR_OK, "erase at last index succeeds");
    ASSERT(arr->count == 2,      "count decrements after tail erase");

    p = array_at(arr, 0, &err); ASSERT(*(int *)p == 20, "index 0 unchanged after tail erase");
    p = array_at(arr, 1, &err); ASSERT(*(int *)p == 40, "index 1 unchanged after tail erase");

    /* ---- erase down to a single element ---- */

    /* [20, 40] -> erase index 0 -> [40] */
    array_erase(arr, 0, &err);
    ASSERT(arr->count == 1,      "count is 1 after erasing down to single element");
    p = array_at(arr, 0, &err);
    ASSERT(*(int *)p == 40, "remaining element has correct value");

    /* ---- erase last remaining element ---- */

    array_erase(arr, 0, &err);
    ASSERT(err.code == ERROR_OK, "erase of last element succeeds");
    ASSERT(arr->count == 0,      "count is 0 after erasing last element");

    err.code = ERROR_OK;
    array_erase(arr, 0, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "erase on newly empty array sets invalid args error");

    /* ---- NULL err pointer is safe ---- */

    value = 1; array_insert(arr, 0, &value, NULL);
    array_erase(arr, 0, NULL);
    ASSERT(1, "erase works fine with NULL err pointer");

    array_destroy(arr);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("array_create",                 test_array_create);
    RUN_TEST("array_destroy",                test_array_destroy);
    RUN_TEST("array_at",                     test_array_at);
    RUN_TEST("array_front",                  test_array_front);
    RUN_TEST("array_back",                   test_array_back);
    RUN_TEST("array_capacity",               test_array_capacity);
    RUN_TEST("array_push_back",              test_array_push_back);
    RUN_TEST("array_push_back_invalid_args", test_array_push_back_invalid_args);
    RUN_TEST("array_insert",                 test_array_insert);
    RUN_TEST("array_erase",                  test_array_erase);

    SUMMARY();
    return failed ? 1 : 0;
}
