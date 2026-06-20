#include <containers.h>
#include <stdio.h>

int main(void) {
    error_t err;
    array_t* arr = array_create(sizeof(int), 0, &err);
    if (!arr) {
        fprintf(stderr, "Error: %s\n", err.msg);
        return 1;
    }

    fprintf(stdout, "Allocated array at %p\n", arr);

    array_destroy(arr);

    return 0;
}


