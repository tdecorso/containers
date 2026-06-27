#include <containers.h>

typedef struct list_node {
    void* data;             
    struct list_node* next; 
    struct list_node* prev;
} list_node_t;

list_node_t* list_node_create(size_t elem_size, error_t* err);

void list_node_destroy(list_node_t* n);
