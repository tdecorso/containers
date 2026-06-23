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

    SUMMARY();
    return failed ? 1 : 0;
}
