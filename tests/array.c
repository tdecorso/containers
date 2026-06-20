#include <containers.h>
#include <stdio.h>

static void error_print(error_t* err) {
    if (!err) return;
    fprintf(stderr, "Error: %s\n", err->msg);
}

int main(void) {
    error_t err;
    array_t* arr = array_create(sizeof(int), 10, &err);
    if (!arr) {
        error_print(&err);
        return 1;
    }

    fprintf(stdout, "Allocated array at %p\n", arr);

    int item = 1;
    array_push_back(arr, &item, NULL);
    item++;
    array_push_back(arr, &item, NULL);
    if (err.code != ERROR_OK) {
        error_print(&err);
        return 1;
    }

    printf("Front item: %d\n", *(int*)array_front(arr, NULL));

    array_destroy(arr);

    return 0;
}

