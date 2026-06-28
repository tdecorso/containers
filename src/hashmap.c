#include <containers.h>
#include <string.h>
#include <stdint.h>

#define HASHMAP_DEFAULT_LOAD_FACTOR  0.75f
#define HASHMAP_DEFAULT_BUCKET_COUNT 8

static void error_create(error_t* err, error_e code, const char* msg) {
    if (!err) return;
    err->code = code;
    size_t len = strlen(msg);
    memcpy(err->msg, msg, len);
    err->msg[len] = '\0';
}

static void* entry_key(void* entry) {
    return entry;
}

static void* entry_value(void* entry, size_t key_size) {
    return (uint8_t*)entry + key_size;
}

hashmap_t* hashmap_create(
        size_t    key_size,
        size_t    value_size,
        size_t    bucket_count,
        hash_fn_t hash,
        cmp_fn_t  cmp,
        error_t*  err) {
    if (key_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Key size cannot be zero.");
        return NULL;
    }
    if (value_size == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Value size cannot be zero.");
        return NULL;
    }
    if (!hash) {
        error_create(err, ERROR_INVALID_ARGS, "Hash function cannot be NULL.");
        return NULL;
    }
    if (!cmp) {
        error_create(err, ERROR_INVALID_ARGS, "Comparison function cannot be NULL.");
        return NULL;
    }

    hashmap_t* map = malloc(sizeof(hashmap_t));
    if (!map) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    if (bucket_count == 0)
        bucket_count = HASHMAP_DEFAULT_BUCKET_COUNT;

    map->buckets = malloc(sizeof(list_t*) * bucket_count);
    if (!map->buckets) {
        free(map);
        error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
        return NULL;
    }

    size_t entry_size = key_size + value_size;
    for (size_t i = 0; i < bucket_count; i++) {
        map->buckets[i] = list_create(entry_size, err);
        if (!map->buckets[i]) {
            for (size_t j = 0; j < i; j++)
                list_destroy(map->buckets[j]);
            free(map->buckets);
            free(map);
            error_create(err, ERROR_OUT_OF_MEMORY, "Memory allocation failed.");
            return NULL;
        }
    }

    map->bucket_count    = bucket_count;
    map->count           = 0;
    map->key_size        = key_size;
    map->value_size      = value_size;
    map->max_load_factor = HASHMAP_DEFAULT_LOAD_FACTOR;
    map->hash            = hash;
    map->cmp             = cmp;

    error_create(err, ERROR_OK, "No error found.");
    return map;
}

void hashmap_destroy(hashmap_t* map) {
    if (!map) return;
    for (size_t i = 0; i < map->bucket_count; i++)
        list_destroy(map->buckets[i]);
    free(map->buckets);
    free(map);
}

static void hashmap_resize(hashmap_t* map, size_t new_bucket_count, error_t* err) {
    if (!map || new_bucket_count == 0) {
        error_create(err, ERROR_INVALID_ARGS, "Invalid resize parameters.");
        return;
    }

    size_t entry_size = map->key_size + map->value_size;

    list_t** new_buckets = malloc(new_bucket_count * sizeof(list_t*));
    if (!new_buckets) {
        error_create(err, ERROR_OUT_OF_MEMORY, "Failed to allocate new buckets.");
        return;
    }

    for (size_t i = 0; i < new_bucket_count; i++) {
        new_buckets[i] = list_create(entry_size, err);
        if (!new_buckets[i]) {
            for (size_t j = 0; j < i; j++)
                list_destroy(new_buckets[j]);
            free(new_buckets);
            error_create(err, ERROR_OUT_OF_MEMORY, "Bucket allocation failed.");
            return;
        }
    }

    list_t**  old_buckets      = map->buckets;
    size_t    old_bucket_count = map->bucket_count;

    map->buckets      = new_buckets;
    map->bucket_count = new_bucket_count;
    map->count        = 0;

    for (size_t i = 0; i < old_bucket_count; i++) {
        list_node_t* node = old_buckets[i]->root;
        while (node) {
            void*  entry = list_node_data(node, NULL);
            size_t index = map->hash(entry_key(entry), map->key_size) % map->bucket_count;

            list_push_back(map->buckets[index], entry, err);
            if (err && err->code != ERROR_OK) {
                for (size_t j = 0; j < new_bucket_count; j++)
                    list_destroy(new_buckets[j]);
                free(new_buckets);
                map->buckets      = old_buckets;
                map->bucket_count = old_bucket_count;
                return;
            }

            map->count++;
            node = list_next(node, NULL);
        }
        list_destroy(old_buckets[i]);
    }

    free(old_buckets);
    error_create(err, ERROR_OK, "No error found.");
}

void hashmap_insert(hashmap_t* map, const void* key, const void* value, error_t* err) {
    if (!map) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL map.");
        return;
    }
    if (!key) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL key.");
        return;
    }
    if (!value) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL value.");
        return;
    }

    size_t  index  = map->hash(key, map->key_size) % map->bucket_count;
    list_t* bucket = map->buckets[index];

    /* update existing key */
    list_node_t* node = bucket->root;
    while (node) {
        void* entry = list_node_data(node, NULL);
        if (map->cmp(entry_key(entry), key, map->key_size) == 0) {
            memcpy(entry_value(entry, map->key_size), value, map->value_size);
            error_create(err, ERROR_OK, "No error found.");
            return;
        }
        node = list_next(node, NULL);
    }

    /* build flat entry on the stack: [key | value] */
    uint8_t entry[map->key_size + map->value_size];
    memcpy(entry_key(entry),                  key,   map->key_size);
    memcpy(entry_value(entry, map->key_size), value, map->value_size);

    list_push_back(bucket, entry, err);
    if (err && err->code != ERROR_OK) return;

    map->count++;

    float load = (float)map->count / (float)map->bucket_count;
    if (load > map->max_load_factor) {
        hashmap_resize(map, map->bucket_count * 2, err);
        if (err && err->code != ERROR_OK) return;
    }

    error_create(err, ERROR_OK, "No error found.");
}

void hashmap_remove(hashmap_t* map, const void* key, void* item_out, error_t* err) {
    if (!map) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL map.");
        return;
    }
    if (!key) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL key.");
        return;
    }

    size_t  index  = map->hash(key, map->key_size) % map->bucket_count;
    list_t* bucket = map->buckets[index];

    list_node_t* node = bucket->root;
    while (node) {
        void* entry = list_node_data(node, NULL);
        if (map->cmp(entry_key(entry), key, map->key_size) == 0) {
            if (item_out) memcpy(item_out, entry_value(entry, map->key_size), map->value_size);
            list_erase(bucket, node, NULL, err);
            map->count--;
            error_create(err, ERROR_OK, "No error found.");
            return;
        }
        node = list_next(node, NULL);
    }

    error_create(err, ERROR_INVALID_ARGS, "Key not found.");
}

void hashmap_get(hashmap_t* map, const void* key, void* item_out, error_t* err) {
    if (!map) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL map.");
        return;
    }
    if (!key) {
        error_create(err, ERROR_INVALID_ARGS, "You passed a NULL key.");
        return;
    }

    size_t  index  = map->hash(key, map->key_size) % map->bucket_count;
    list_t* bucket = map->buckets[index];

    list_node_t* node = bucket->root;
    while (node) {
        void* entry = list_node_data(node, NULL);
        if (map->cmp(entry_key(entry), key, map->key_size) == 0) {
            if (item_out)
                memcpy(item_out, entry_value(entry, map->key_size), map->value_size);
            error_create(err, ERROR_OK, "No error found.");
            return;
        }
        node = list_next(node, NULL);
    }

    error_create(err, ERROR_INVALID_ARGS, "Key not found.");
}
