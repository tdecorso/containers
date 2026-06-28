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
/* test helpers: custom struct + hash + cmp                          */
/* ---------------------------------------------------------------- */

typedef struct {
    int   id;
    float score;
    char  label[32];
} record_t;

static size_t hash_int(const void* key, size_t key_size) {
    (void)key_size;
    size_t k = (size_t)(*(const int *)key);
    /* FNV-1a inspired mixing */
    k ^= k >> 16;
    k *= 0x45d9f3b;
    k ^= k >> 16;
    return k;
}

static int cmp_int(const void* a, const void* b, size_t key_size) {
    (void)key_size;
    return *(const int *)a - *(const int *)b;
}

static size_t hash_str(const void* key, size_t key_size) {
    const unsigned char *p = (const unsigned char *)key;
    size_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < key_size && p[i] != '\0'; i++) {
        hash ^= p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static int cmp_str(const void* a, const void* b, size_t key_size) {
    return strncmp((const char *)a, (const char *)b, key_size);
}

/* ---------------------------------------------------------------- */
/* hashmap_create / hashmap_destroy                                  */
/* ---------------------------------------------------------------- */

int test_hashmap_create(void) {
    error_t     err;
    hashmap_t  *map;

    map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    ASSERT(map != NULL,                       "hashmap_create returns valid pointer");
    ASSERT(err.code == ERROR_OK,              "no error on valid creation");
    ASSERT(map->count == 0,                   "count starts at zero");
    ASSERT(map->key_size == sizeof(int),      "key_size stored correctly");
    ASSERT(map->value_size == sizeof(record_t), "value_size stored correctly");
    ASSERT(map->buckets != NULL,              "bucket array is initialized");
    ASSERT(map->bucket_count > 0,            "bucket_count is positive");
    hashmap_destroy(map);

    /* zero bucket_count: should use default */
    map = hashmap_create(sizeof(int), sizeof(record_t), 0, hash_int, cmp_int, &err);
    ASSERT(map != NULL,          "hashmap_create with bucket_count=0 uses default");
    ASSERT(err.code == ERROR_OK, "no error when bucket_count=0");
    hashmap_destroy(map);

    /* invalid args */
    map = hashmap_create(0, sizeof(record_t), 16, hash_int, cmp_int, &err);
    ASSERT(map == NULL,                    "hashmap_create returns NULL on zero key_size");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero key_size");

    map = hashmap_create(sizeof(int), 0, 16, hash_int, cmp_int, &err);
    ASSERT(map == NULL,                    "hashmap_create returns NULL on zero value_size");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on zero value_size");

    map = hashmap_create(sizeof(int), sizeof(record_t), 16, NULL, cmp_int, &err);
    ASSERT(map == NULL,                    "hashmap_create returns NULL on NULL hash fn");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on NULL hash fn");

    map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, NULL, &err);
    ASSERT(map == NULL,                    "hashmap_create returns NULL on NULL cmp fn");
    ASSERT(err.code == ERROR_INVALID_ARGS, "invalid args error on NULL cmp fn");

    /* NULL err pointer */
    map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, NULL);
    ASSERT(map != NULL, "hashmap_create works fine with NULL err pointer");
    hashmap_destroy(map);

    return 0;
}

int test_hashmap_destroy(void) {
    error_t     err;
    hashmap_t  *map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    int         key = 1;
    record_t    rec = { .id = 1, .score = 9.5f, .label = "alpha" };

    ASSERT(map != NULL, "hashmap_create succeeds before destroy test");

    /* destroy empty map */
    hashmap_destroy(map);
    ASSERT(1, "hashmap_destroy does not crash on empty map");

    /* destroy populated map: verifies no leaks in deep cleanup */
    map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    ASSERT(map != NULL, "hashmap_create succeeds for populated destroy test");

    for (int i = 0; i < 32; i++) {
        rec.id = i;
        hashmap_insert(map, &i, &rec, &err);
    }
    ASSERT(map->count == 32, "32 entries inserted before destroy");

    hashmap_destroy(map);
    ASSERT(1, "hashmap_destroy does not crash on populated map");

    hashmap_destroy(NULL);
    ASSERT(1, "hashmap_destroy is safe to call on NULL");

    (void)key;
    return 0;
}

/* ---------------------------------------------------------------- */
/* hashmap_insert                                                    */
/* ---------------------------------------------------------------- */

int test_hashmap_insert(void) {
    error_t     err;
    hashmap_t  *map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    record_t    rec, out;
    int         key;

    ASSERT(map != NULL, "hashmap_create succeeds before hashmap_insert test");

    /* invalid args */
    key = 1;
    rec = (record_t){ .id = 1, .score = 1.0f, .label = "a" };

    err.code = ERROR_OK;
    hashmap_insert(NULL, &key, &rec, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert on NULL map sets invalid args error");

    err.code = ERROR_OK;
    hashmap_insert(map, NULL, &rec, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert with NULL key sets invalid args error");

    err.code = ERROR_OK;
    hashmap_insert(map, &key, NULL, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "insert with NULL value sets invalid args error");

    /* basic insert */
    key = 42;
    rec = (record_t){ .id = 42, .score = 3.14f, .label = "hello" };
    hashmap_insert(map, &key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "insert of new key succeeds");
    ASSERT(map->count == 1,      "count is 1 after first insert");

    hashmap_get(map, &key, &out, &err);
    ASSERT(err.code == ERROR_OK,          "get after insert succeeds");
    ASSERT(out.id == 42,                  "inserted record id is correct");
    ASSERT(out.score == 3.14f,            "inserted record score is correct");
    ASSERT(strcmp(out.label, "hello") == 0, "inserted record label is correct");

    /* insert multiple distinct keys */
    for (int i = 0; i < 10; i++) {
        rec = (record_t){ .id = i, .score = (float)i, .label = "x" };
        hashmap_insert(map, &i, &rec, &err);
        ASSERT(err.code == ERROR_OK, "insert of distinct key succeeds");
    }
    ASSERT(map->count == 11, "count is 11 after inserting key=42 plus 10 more");

    /* update existing key */
    key = 42;
    rec = (record_t){ .id = 99, .score = 2.71f, .label = "updated" };
    hashmap_insert(map, &key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "insert on existing key succeeds");
    ASSERT(map->count == 11,     "count unchanged after updating existing key");

    hashmap_get(map, &key, &out, &err);
    ASSERT(out.id == 99,                    "updated record id is correct");
    ASSERT(out.score == 2.71f,              "updated record score is correct");
    ASSERT(strcmp(out.label, "updated") == 0, "updated record label is correct");

    /* trigger resize: insert enough entries to exceed load factor */
    for (int i = 100; i < 200; i++) {
        rec = (record_t){ .id = i, .score = (float)i, .label = "bulk" };
        hashmap_insert(map, &i, &rec, &err);
        ASSERT(err.code == ERROR_OK, "bulk insert succeeds during resize");
    }
    ASSERT(map->count == 111, "count is correct after bulk insert and resize");

    /* verify all entries survive resize */
    for (int i = 100; i < 200; i++) {
        hashmap_get(map, &i, &out, &err);
        ASSERT(err.code == ERROR_OK, "get after resize succeeds for all bulk entries");
        ASSERT(out.id == i,          "bulk entry value is correct after resize");
    }

    /* NULL err pointer */
    key = 999;
    rec = (record_t){ .id = 999, .score = 0.0f, .label = "null_err" };
    hashmap_insert(map, &key, &rec, NULL);
    ASSERT(1, "insert works fine with NULL err pointer");

    hashmap_destroy(map);
    return 0;
}

/* ---------------------------------------------------------------- */
/* hashmap_get                                                       */
/* ---------------------------------------------------------------- */

int test_hashmap_get(void) {
    error_t     err;
    hashmap_t  *map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    record_t    rec, out;
    int         key;

    ASSERT(map != NULL, "hashmap_create succeeds before hashmap_get test");

    /* populate */
    for (int i = 0; i < 5; i++) {
        rec = (record_t){ .id = i, .score = (float)i * 0.5f, .label = "rec" };
        snprintf(rec.label, sizeof(rec.label), "rec_%d", i);
        hashmap_insert(map, &i, &rec, &err);
    }

    /* invalid args */
    err.code = ERROR_OK;
    hashmap_get(NULL, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "get on NULL map sets invalid args error");

    key = 0;
    err.code = ERROR_OK;
    hashmap_get(map, NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "get with NULL key sets invalid args error");

    /* key not found */
    key = 999;
    err.code = ERROR_OK;
    hashmap_get(map, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "get with missing key sets invalid args error");

    /* get with item_out: verify all fields */
    for (int i = 0; i < 5; i++) {
        hashmap_get(map, &i, &out, &err);
        ASSERT(err.code == ERROR_OK, "get of existing key succeeds");
        ASSERT(out.id == i,          "retrieved record id is correct");
        ASSERT(out.score == (float)i * 0.5f, "retrieved record score is correct");
        char expected_label[32];
        snprintf(expected_label, sizeof(expected_label), "rec_%d", i);
        ASSERT(strcmp(out.label, expected_label) == 0, "retrieved record label is correct");
    }

    /* get with NULL item_out: presence check, ERROR_OK */
    key = 2;
    hashmap_get(map, &key, NULL, &err);
    ASSERT(err.code == ERROR_OK, "get with NULL item_out and existing key sets ERROR_OK");

    /* get does not modify the map */
    ASSERT(map->count == 5, "map count unchanged after multiple gets");

    /* NULL err pointer */
    key = 0;
    hashmap_get(map, &key, &out, NULL);
    ASSERT(out.id == 0, "get works fine with NULL err pointer");

    hashmap_destroy(map);
    return 0;
}

/* ---------------------------------------------------------------- */
/* hashmap_remove                                                    */
/* ---------------------------------------------------------------- */

int test_hashmap_remove(void) {
    error_t     err;
    hashmap_t  *map = hashmap_create(sizeof(int), sizeof(record_t), 16, hash_int, cmp_int, &err);
    record_t    rec, out;
    int         key;

    ASSERT(map != NULL, "hashmap_create succeeds before hashmap_remove test");

    /* invalid args */
    key = 1;
    err.code = ERROR_OK;
    hashmap_remove(NULL, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "remove on NULL map sets invalid args error");

    err.code = ERROR_OK;
    hashmap_remove(map, NULL, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "remove with NULL key sets invalid args error");

    /* remove from empty map */
    err.code = ERROR_OK;
    hashmap_remove(map, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "remove on empty map sets invalid args error");

    /* populate: keys 0..4 */
    for (int i = 0; i < 5; i++) {
        rec = (record_t){ .id = i, .score = (float)i, .label = "r" };
        snprintf(rec.label, sizeof(rec.label), "rec_%d", i);
        hashmap_insert(map, &i, &rec, &err);
    }
    ASSERT(map->count == 5, "map has 5 entries before remove tests");

    /* remove with item_out */
    key = 2;
    hashmap_remove(map, &key, &out, &err);
    ASSERT(err.code == ERROR_OK,              "remove of existing key succeeds");
    ASSERT(map->count == 4,                   "count decrements after remove");
    ASSERT(out.id == 2,                       "item_out receives correct id");
    ASSERT(out.score == 2.0f,                 "item_out receives correct score");
    ASSERT(strcmp(out.label, "rec_2") == 0,   "item_out receives correct label");

    /* removed key is no longer accessible */
    err.code = ERROR_OK;
    hashmap_get(map, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "get after remove sets invalid args error");

    /* remove with NULL item_out */
    key = 3;
    hashmap_remove(map, &key, NULL, &err);
    ASSERT(err.code == ERROR_OK, "remove with NULL item_out succeeds");
    ASSERT(map->count == 3,      "count decrements after remove with NULL item_out");

    err.code = ERROR_OK;
    hashmap_get(map, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "removed key is no longer accessible");

    /* remove already-removed key */
    err.code = ERROR_OK;
    hashmap_remove(map, &key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "remove of already-removed key sets invalid args error");
    ASSERT(map->count == 3, "count unchanged after failed remove");

    /* remaining entries are intact */
    int remaining[] = { 0, 1, 4 };
    for (int i = 0; i < 3; i++) {
        hashmap_get(map, &remaining[i], &out, &err);
        ASSERT(err.code == ERROR_OK,         "remaining entry is still accessible after removes");
        ASSERT(out.id == remaining[i],       "remaining entry has correct value after removes");
    }

    /* remove all remaining entries */
    for (int i = 0; i < 3; i++) {
        hashmap_remove(map, &remaining[i], NULL, &err);
        ASSERT(err.code == ERROR_OK, "remove of remaining entry succeeds");
    }
    ASSERT(map->count == 0, "count is 0 after removing all entries");

    /* NULL err pointer */
    key = 0;
    rec = (record_t){ .id = 0, .score = 0.0f, .label = "tmp" };
    hashmap_insert(map, &key, &rec, NULL);
    hashmap_remove(map, &key, NULL, NULL);
    ASSERT(1, "remove works fine with NULL err pointer");

    hashmap_destroy(map);
    return 0;
}

/* ---------------------------------------------------------------- */
/* hashmap with string keys                                          */
/* ---------------------------------------------------------------- */

int test_hashmap_string_keys(void) {
    error_t     err;
    hashmap_t  *map = hashmap_create(32, sizeof(record_t), 16, hash_str, cmp_str, &err);
    record_t    rec, out;
    char        key[32];

    ASSERT(map != NULL, "hashmap_create with string keys returns valid pointer");

    /* insert with string keys */
    strncpy(key, "alpha", sizeof(key));
    rec = (record_t){ .id = 1, .score = 1.1f, .label = "alpha_val" };
    hashmap_insert(map, key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "insert with string key 'alpha' succeeds");

    strncpy(key, "beta", sizeof(key));
    rec = (record_t){ .id = 2, .score = 2.2f, .label = "beta_val" };
    hashmap_insert(map, key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "insert with string key 'beta' succeeds");

    strncpy(key, "gamma", sizeof(key));
    rec = (record_t){ .id = 3, .score = 3.3f, .label = "gamma_val" };
    hashmap_insert(map, key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "insert with string key 'gamma' succeeds");

    ASSERT(map->count == 3, "count is 3 after three string-key inserts");

    /* get by string key */
    strncpy(key, "alpha", sizeof(key));
    hashmap_get(map, key, &out, &err);
    ASSERT(err.code == ERROR_OK,                "get by string key 'alpha' succeeds");
    ASSERT(out.id == 1,                         "retrieved record id is correct");
    ASSERT(strcmp(out.label, "alpha_val") == 0, "retrieved record label is correct");

    strncpy(key, "gamma", sizeof(key));
    hashmap_get(map, key, &out, &err);
    ASSERT(err.code == ERROR_OK,                "get by string key 'gamma' succeeds");
    ASSERT(out.id == 3,                         "retrieved record id is correct");

    /* missing key */
    strncpy(key, "delta", sizeof(key));
    err.code = ERROR_OK;
    hashmap_get(map, key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "get with missing string key sets invalid args error");

    /* update by string key */
    strncpy(key, "beta", sizeof(key));
    rec = (record_t){ .id = 99, .score = 9.9f, .label = "beta_updated" };
    hashmap_insert(map, key, &rec, &err);
    ASSERT(err.code == ERROR_OK, "update by string key 'beta' succeeds");
    ASSERT(map->count == 3,      "count unchanged after update");

    hashmap_get(map, key, &out, &err);
    ASSERT(out.id == 99,                          "updated record id is correct");
    ASSERT(strcmp(out.label, "beta_updated") == 0, "updated record label is correct");

    /* remove by string key */
    strncpy(key, "alpha", sizeof(key));
    hashmap_remove(map, key, &out, &err);
    ASSERT(err.code == ERROR_OK,                "remove by string key 'alpha' succeeds");
    ASSERT(out.id == 1,                         "item_out from string key remove is correct");
    ASSERT(map->count == 2,                     "count decrements after string key remove");

    err.code = ERROR_OK;
    hashmap_get(map, key, &out, &err);
    ASSERT(err.code == ERROR_INVALID_ARGS, "removed string key is no longer accessible");

    hashmap_destroy(map);
    return 0;
}

int main(void) {
    printf(C_BOLD C_YELLOW "\n  containers.h — test suite\n\n" C_RESET);

    RUN_TEST("hashmap_create",      test_hashmap_create);
    RUN_TEST("hashmap_destroy",     test_hashmap_destroy);
    RUN_TEST("hashmap_insert",      test_hashmap_insert);
    RUN_TEST("hashmap_get",         test_hashmap_get);
    RUN_TEST("hashmap_remove",      test_hashmap_remove);
    RUN_TEST("hashmap_string_keys", test_hashmap_string_keys);

    SUMMARY();
    return failed ? 1 : 0;
}
