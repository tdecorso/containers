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
/* queue_create / queue_destroy                                      */
/* ---------------------------------------------------------------- */

int test_queue_create(void) {
    error_t   err;
    queue_t  *queue;

    queue = queue_create(sizeof(int), &err);
    ASSERT(queue != NULL,         "queue_create returns valid pointer");
    ASSERT(err.code == ERROR_OK,  "no error on valid creation");
    queue_destroy(queue);

    queue = queue_create(0, &err);
    ASSERT(queue == NULL,                   "queue_create returns NULL on zero elem_size");
    ASSERT(err.code == ERROR_INVALID_ARGS,  "invalid args error on zero elem_size");

    queue = queue_create(sizeof(int), NULL);
    ASSERT(queue != NULL, "queue_create works fine with NULL err pointer");
    queue_destroy(queue);

    return 0;
}

int test_queue_destroy(void) {
    error_t   err;
    queue_t  *queue = queue_create(sizeof(int), &err);

    ASSERT(queue != NULL, "queue_create succeeds before destroy test");

    queue_destroy(queue);
    ASSERT(1, "queue_destroy does not crash on a valid queue");

    queue_destroy(NULL);
    ASSERT(1, "queue_destroy is safe to call on NULL");

    return 0;
}

/* ---------------------------------------------------------------- */
/* queue_is_empty / queue_size                                       */
/* ---------------------------------------------------------------- */

int test_queue_capacity(void) {
    error_t   err;
    queue_t  *queue = queue_create(sizeof(int), &err);
    int       value = 42;

    ASSERT(queue != NULL, "queue_create succeeds before capacity test");

    /* empty state */
    ASSERT(queue_is_empty(queue, &err) == true, "newly created queue is empty");
    ASSERT(err.code == ERROR_OK,                "no error on empty queue_is_empty");
    ASSERT(queue_size(queue, &err) == 0,        "size is 0 on empty queue");
    ASSERT(err.code == ERROR_OK,                "no error on empty queue_size");

    /* after one push */
    queue_push(queue, &value, &err);
    ASSERT(queue_is_empty(queue, &err) == false, "queue is not empty after push");
    ASSERT(queue_size(queue, &err) == 1,         "size is 1 after one push");

    /* after a second push */
    queue_push(queue, &value, &err);
    ASSERT(queue_size(queue, &err) == 2, "size is 2 after two pushes");

    /* after one pop */
    queue_pop(queue, NULL, &err);
    ASSERT(queue_size(queue, &err) == 1,         "size decrements after pop");
    ASSERT(queue_is_empty(queue, &err) == false, "queue not empty with one element remaining");

    /* after popping last element */
    queue_pop(queue, NULL, &err);
    ASSERT(queue_size(queue, &err) == 0,        "size is 0 after popping all elements");
    ASSERT(queue_is_empty(queue, &err) == true, "queue is empty after popping all elements");

    /* invalid args */
    err.code = ERROR_OK;
    queue_is_empty(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_is_empty on NULL queue sets invalid args error");

    err.code = ERROR_OK;
    queue_size(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_size on NULL queue sets invalid args error");

    /* NULL err pointer */
    ASSERT(queue_is_empty(queue, NULL) == true, "queue_is_empty works fine with NULL err pointer");
    ASSERT(queue_size(queue, NULL) == 0,        "queue_size works fine with NULL err pointer");

    queue_destroy(queue);
    return 0;
}

/* ---------------------------------------------------------------- */
/* queue_front / queue_back                                          */
/* ---------------------------------------------------------------- */

int test_queue_access(void) {
    error_t   err;
    queue_t  *queue = queue_create(sizeof(int), &err);
    void     *p;
    int       a = 10, b = 20, c = 30;

    ASSERT(queue != NULL, "queue_create succeeds before access test");

    /* empty queue: NULL, ERROR_OK */
    p = queue_front(queue, &err);
    ASSERT(p == NULL,            "queue_front on empty queue returns NULL");
    ASSERT(err.code == ERROR_OK, "queue_front on empty queue does not set an error");

    p = queue_back(queue, &err);
    ASSERT(p == NULL,            "queue_back on empty queue returns NULL");
    ASSERT(err.code == ERROR_OK, "queue_back on empty queue does not set an error");

    /* single element: front and back point to same value */
    queue_push(queue, &a, &err);
    p = queue_front(queue, &err);
    ASSERT(p != NULL,          "queue_front returns valid pointer after one push");
    ASSERT(*(int *)p == 10,    "queue_front returns correct value after one push");

    p = queue_back(queue, &err);
    ASSERT(p != NULL,          "queue_back returns valid pointer after one push");
    ASSERT(*(int *)p == 10,    "queue_back returns correct value after one push");

    /* multiple elements: front is oldest, back is newest */
    queue_push(queue, &b, &err);
    queue_push(queue, &c, &err);

    p = queue_front(queue, &err);
    ASSERT(err.code == ERROR_OK, "no error on queue_front with multiple elements");
    ASSERT(*(int *)p == 10,      "queue_front returns oldest element");

    p = queue_back(queue, &err);
    ASSERT(err.code == ERROR_OK, "no error on queue_back with multiple elements");
    ASSERT(*(int *)p == 30,      "queue_back returns newest element");

    /* front updates after pop */
    queue_pop(queue, NULL, &err);
    p = queue_front(queue, &err);
    ASSERT(*(int *)p == 20, "queue_front updates correctly after pop");

    /* back unchanged after pop from front */
    p = queue_back(queue, &err);
    ASSERT(*(int *)p == 30, "queue_back unchanged after pop from front");

    /* invalid args */
    err.code = ERROR_OK;
    queue_front(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_front on NULL queue sets invalid args error");

    err.code = ERROR_OK;
    queue_back(NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_back on NULL queue sets invalid args error");

    /* NULL err pointer */
    p = queue_front(queue, NULL);
    ASSERT(p != NULL, "queue_front works fine with NULL err pointer");
    p = queue_back(queue, NULL);
    ASSERT(p != NULL, "queue_back works fine with NULL err pointer");

    queue_destroy(queue);
    return 0;
}

/* ---------------------------------------------------------------- */
/* queue_push / queue_pop                                            */
/* ---------------------------------------------------------------- */

int test_queue_push(void) {
    error_t   err;
    queue_t  *queue = queue_create(sizeof(int), &err);
    int       a = 10, b = 20, c = 30;

    ASSERT(queue != NULL, "queue_create succeeds before queue_push test");

    /* invalid args */
    err.code = ERROR_OK;
    queue_push(NULL, &a, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_push on NULL queue sets invalid args error");

    err.code = ERROR_OK;
    queue_push(queue, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_push with NULL item sets invalid args error");

    /* push and verify FIFO order via front/back */
    queue_push(queue, &a, &err);
    ASSERT(err.code == ERROR_OK,                  "first queue_push succeeds");
    ASSERT(queue_size(queue, &err) == 1,          "size is 1 after first push");
    ASSERT(*(int *)queue_front(queue, &err) == 10, "front is 10 after first push");
    ASSERT(*(int *)queue_back(queue, &err)  == 10, "back is 10 after first push");

    queue_push(queue, &b, &err);
    ASSERT(err.code == ERROR_OK,                  "second queue_push succeeds");
    ASSERT(queue_size(queue, &err) == 2,          "size is 2 after second push");
    ASSERT(*(int *)queue_front(queue, &err) == 10, "front unchanged after second push");
    ASSERT(*(int *)queue_back(queue, &err)  == 20, "back updated after second push");

    queue_push(queue, &c, &err);
    ASSERT(err.code == ERROR_OK,                  "third queue_push succeeds");
    ASSERT(queue_size(queue, &err) == 3,          "size is 3 after third push");
    ASSERT(*(int *)queue_front(queue, &err) == 10, "front unchanged after third push");
    ASSERT(*(int *)queue_back(queue, &err)  == 30, "back updated after third push");

    /* NULL err pointer */
    queue_push(queue, &a, NULL);
    ASSERT(1, "queue_push works fine with NULL err pointer");

    queue_destroy(queue);
    return 0;
}

int test_queue_pop(void) {
    error_t   err;
    queue_t  *queue = queue_create(sizeof(int), &err);
    int       a = 10, b = 20, c = 30, out;

    ASSERT(queue != NULL, "queue_create succeeds before queue_pop test");

    /* invalid args */
    err.code = ERROR_OK;
    queue_pop(NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_pop on NULL queue sets invalid args error");

    /* pop on empty queue */
    err.code = ERROR_OK;
    queue_pop(queue, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_pop on empty queue sets invalid args error");

    /* populate: [10, 20, 30] */
    queue_push(queue, &a, &err);
    queue_push(queue, &b, &err);
    queue_push(queue, &c, &err);

    /* pop with item_out: verify FIFO order */
    queue_pop(queue, &out, &err);
    ASSERT(err.code == ERROR_OK, "first queue_pop succeeds");
    ASSERT(out == 10,            "first popped element is 10");
    ASSERT(queue_size(queue, &err) == 2, "size decrements after first pop");

    queue_pop(queue, &out, &err);
    ASSERT(err.code == ERROR_OK, "second queue_pop succeeds");
    ASSERT(out == 20,            "second popped element is 20");
    ASSERT(queue_size(queue, &err) == 1, "size decrements after second pop");

    /* pop last element: root and tail should reset */
    queue_pop(queue, &out, &err);
    ASSERT(err.code == ERROR_OK,                "third queue_pop succeeds");
    ASSERT(out == 30,                           "third popped element is 30");
    ASSERT(queue_size(queue, &err) == 0,        "size is 0 after popping all elements");
    ASSERT(queue_is_empty(queue, &err) == true, "queue is empty after popping all elements");

    /* pop on newly empty queue */
    err.code = ERROR_OK;
    queue_pop(queue, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "queue_pop on newly empty queue sets invalid args error");

    /* pop with NULL item_out */
    queue_push(queue, &a, &err);
    queue_pop(queue, NULL, &err);
    ASSERT(err.code == ERROR_OK,                "queue_pop with NULL item_out succeeds");
    ASSERT(queue_is_empty(queue, &err) == true, "queue is empty after pop with NULL item_out");

    /* NULL err pointer */
    queue_push(queue, &a, NULL);
    queue_pop(queue, &out, NULL);
    ASSERT(1, "queue_pop works fine with NULL err pointer");

    queue_destroy(queue);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("queue_create",   test_queue_create);
    RUN_TEST("queue_destroy",  test_queue_destroy);
    RUN_TEST("queue_capacity", test_queue_capacity);
    RUN_TEST("queue_access",   test_queue_access);
    RUN_TEST("queue_push",     test_queue_push);
    RUN_TEST("queue_pop",      test_queue_pop);

    SUMMARY();
    return failed ? 1 : 0;
}
