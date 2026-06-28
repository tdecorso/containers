#include <containers.h>
#include <string.h>

#define FNV_OFFSET_BASIS 14695981039346656037ULL
#define FNV_PRIME        1099511628211ULL

size_t hash_bytes(const void* key, size_t key_size) {
    const uint8_t* p    = (const uint8_t*)key;
    size_t         hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= p[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

size_t hash_string(const void* key, size_t key_size) {
    const uint8_t* p    = (const uint8_t*)key;
    size_t         hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < key_size && p[i] != '\0'; i++) {
        hash ^= p[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

size_t hash_int32(const void* key, size_t key_size) {
    (void)key_size;
    uint32_t k = *(const uint32_t*)key;
    k ^= k >> 16;
    k *= 0x45d9f3b;
    k ^= k >> 16;
    return (size_t)k;
}

size_t hash_int64(const void* key, size_t key_size) {
    (void)key_size;
    uint64_t k = *(const uint64_t*)key;
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return (size_t)k;
}

int cmp_bytes(const void* a, const void* b, size_t key_size) {
    return memcmp(a, b, key_size);
}

int cmp_string(const void* a, const void* b, size_t key_size) {
    return strncmp((const char*)a, (const char*)b, key_size);
}
