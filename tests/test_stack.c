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
/* stack_create / stack_destroy                                      */
/* ---------------------------------------------------------------- */

int test_stack_create(void) {
    error_t   err;
    stack_t  *stack;

    stack = stack_create(sizeof(int), &err);
    ASSERT(stack != NULL,        "stack_create returns valid pointer");
    ASSERT(err.code == ERROR_OK, "no error on valid creation");
    ASSERT(stack->arr != NULL,   "internal array is initialized");
    stack_destroy(stack);

    stack = stack_create(0, &err);
    ASSERT(stack == NULL,                  "stack_create returns NULL on zero elem_size");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero elem_size");

    stack = stack_create(sizeof(int), NULL);
    ASSERT(stack != NULL, "stack_create works fine with NULL err pointer");
    stack_destroy(stack);

    return 0;
}

int test_stack_destroy(void) {
    error_t   err;
    stack_t  *stack = stack_create(sizeof(int), &err);

    ASSERT(stack != NULL, "stack_create succeeds before destroy test");

    stack_destroy(stack);
    ASSERT(1, "stack_destroy does not crash on a valid stack");

    stack_destroy(NULL);
    ASSERT(1, "stack_destroy is safe to call on NULL");

    return 0;
}

/* ---------------------------------------------------------------- */
/* stack_is_empty / stack_size                                       */
/* ---------------------------------------------------------------- */

int test_stack_capacity(void) {
    error_t   err;
    stack_t  *stack = stack_create(sizeof(int), &err);
    int       value = 42;

    ASSERT(stack != NULL, "stack_create succeeds before capacity test");

    /* empty state */
    ASSERT(stack_is_empty(stack, &err) == true, "newly created stack is empty");
    ASSERT(err.code == ERROR_OK,                "no error on empty stack_is_empty");
    ASSERT(stack_size(stack, &err) == 0,        "size is 0 on empty stack");
    ASSERT(err.code == ERROR_OK,                "no error on empty stack_size");

    /* after one push */
    stack_push(stack, &value, &err);
    ASSERT(stack_is_empty(stack, &err) == false, "stack is not empty after push");
    ASSERT(stack_size(stack, &err) == 1,         "size is 1 after one push");

    /* after a second push */
    stack_push(stack, &value, &err);
    ASSERT(stack_size(stack, &err) == 2, "size is 2 after two pushes");

    /* after one pop */
    stack_pop(stack, NULL, &err);
    ASSERT(stack_size(stack, &err) == 1,         "size decrements after pop");
    ASSERT(stack_is_empty(stack, &err) == false, "stack not empty with one element remaining");

    /* after popping last element */
    stack_pop(stack, NULL, &err);
    ASSERT(stack_size(stack, &err) == 0,        "size is 0 after popping all elements");
    ASSERT(stack_is_empty(stack, &err) == true, "stack is empty after popping all elements");

    /* invalid args */
    err.code = ERROR_OK;
    stack_is_empty(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_is_empty on NULL stack sets invalid args error");

    err.code = ERROR_OK;
    stack_size(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_size on NULL stack sets invalid args error");

    /* NULL err pointer */
    ASSERT(stack_is_empty(stack, NULL) == true, "stack_is_empty works fine with NULL err pointer");
    ASSERT(stack_size(stack, NULL) == 0,        "stack_size works fine with NULL err pointer");

    stack_destroy(stack);
    return 0;
}

/* ---------------------------------------------------------------- */
/* stack_top                                                         */
/* ---------------------------------------------------------------- */

int test_stack_top(void) {
    error_t   err;
    stack_t  *stack = stack_create(sizeof(int), &err);
    void     *p;
    int       a = 10, b = 20, c = 30;

    ASSERT(stack != NULL, "stack_create succeeds before stack_top test");

    /* empty stack: NULL, ERROR_OK */
    p = stack_top(stack, &err);
    ASSERT(p == NULL,            "stack_top on empty stack returns NULL");
    ASSERT(err.code == ERROR_OK, "stack_top on empty stack does not set an error");

    /* single element */
    stack_push(stack, &a, &err);
    p = stack_top(stack, &err);
    ASSERT(p != NULL,         "stack_top returns valid pointer after one push");
    ASSERT(*(int *)p == 10,   "stack_top returns correct value after one push");

    /* top updates as new elements are pushed */
    stack_push(stack, &b, &err);
    p = stack_top(stack, &err);
    ASSERT(*(int *)p == 20, "stack_top updates to newest element after second push");

    stack_push(stack, &c, &err);
    p = stack_top(stack, &err);
    ASSERT(*(int *)p == 30, "stack_top updates to newest element after third push");

    /* top updates after pop */
    stack_pop(stack, NULL, &err);
    p = stack_top(stack, &err);
    ASSERT(*(int *)p == 20, "stack_top updates correctly after pop");

    stack_pop(stack, NULL, &err);
    p = stack_top(stack, &err);
    ASSERT(*(int *)p == 10, "stack_top updates correctly after second pop");

    /* invalid args */
    err.code = ERROR_OK;
    stack_top(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_top on NULL stack sets invalid args error");

    /* NULL err pointer */
    p = stack_top(stack, NULL);
    ASSERT(p != NULL, "stack_top works fine with NULL err pointer");

    stack_destroy(stack);
    return 0;
}

/* ---------------------------------------------------------------- */
/* stack_push / stack_pop                                            */
/* ---------------------------------------------------------------- */

int test_stack_push(void) {
    error_t   err;
    stack_t  *stack = stack_create(sizeof(int), &err);
    int       a = 10, b = 20, c = 30;

    ASSERT(stack != NULL, "stack_create succeeds before stack_push test");

    /* invalid args */
    err.code = ERROR_OK;
    stack_push(NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_push on NULL stack sets invalid args error");

    err.code = ERROR_OK;
    stack_push(stack, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_push with NULL item sets invalid args error");

    /* push and verify LIFO order via top */
    stack_push(stack, &a, &err);
    ASSERT(err.code == ERROR_OK,                 "first stack_push succeeds");
    ASSERT(stack_size(stack, &err) == 1,         "size is 1 after first push");
    ASSERT(*(int *)stack_top(stack, &err) == 10, "top is 10 after first push");

    stack_push(stack, &b, &err);
    ASSERT(err.code == ERROR_OK,                 "second stack_push succeeds");
    ASSERT(stack_size(stack, &err) == 2,         "size is 2 after second push");
    ASSERT(*(int *)stack_top(stack, &err) == 20, "top is 20 after second push");

    stack_push(stack, &c, &err);
    ASSERT(err.code == ERROR_OK,                 "third stack_push succeeds");
    ASSERT(stack_size(stack, &err) == 3,         "size is 3 after third push");
    ASSERT(*(int *)stack_top(stack, &err) == 30, "top is 30 after third push");

    /* NULL err pointer */
    stack_push(stack, &a, NULL);
    ASSERT(1, "stack_push works fine with NULL err pointer");

    stack_destroy(stack);
    return 0;
}

int test_stack_pop(void) {
    error_t   err;
    stack_t  *stack = stack_create(sizeof(int), &err);
    int       a = 10, b = 20, c = 30, out;

    ASSERT(stack != NULL, "stack_create succeeds before stack_pop test");

    /* invalid args */
    err.code = ERROR_OK;
    stack_pop(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_pop on NULL stack sets invalid args error");

    /* pop on empty stack */
    err.code = ERROR_OK;
    stack_pop(stack, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_pop on empty stack sets invalid args error");

    /* populate: push 10, 20, 30 */
    stack_push(stack, &a, &err);
    stack_push(stack, &b, &err);
    stack_push(stack, &c, &err);

    /* pop with item_out: verify LIFO order */
    stack_pop(stack, &out, &err);
    ASSERT(err.code == ERROR_OK,         "first stack_pop succeeds");
    ASSERT(out == 30,                    "first popped element is 30 (LIFO)");
    ASSERT(stack_size(stack, &err) == 2, "size decrements after first pop");

    stack_pop(stack, &out, &err);
    ASSERT(err.code == ERROR_OK,         "second stack_pop succeeds");
    ASSERT(out == 20,                    "second popped element is 20 (LIFO)");
    ASSERT(stack_size(stack, &err) == 1, "size decrements after second pop");

    /* pop last element with NULL item_out */
    stack_pop(stack, NULL, &err);
    ASSERT(err.code == ERROR_OK,                "stack_pop with NULL item_out succeeds");
    ASSERT(stack_size(stack, &err) == 0,        "size is 0 after popping all elements");
    ASSERT(stack_is_empty(stack, &err) == true, "stack is empty after popping all elements");

    /* pop on newly empty stack */
    err.code = ERROR_OK;
    stack_pop(stack, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "stack_pop on newly empty stack sets invalid args error");

    /* NULL err pointer */
    stack_push(stack, &a, NULL);
    stack_pop(stack, &out, NULL);
    ASSERT(1, "stack_pop works fine with NULL err pointer");

    stack_destroy(stack);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("stack_create",   test_stack_create);
    RUN_TEST("stack_destroy",  test_stack_destroy);
    RUN_TEST("stack_capacity", test_stack_capacity);
    RUN_TEST("stack_top",      test_stack_top);
    RUN_TEST("stack_push",     test_stack_push);
    RUN_TEST("stack_pop",      test_stack_pop);

    SUMMARY();
    return failed ? 1 : 0;
}
