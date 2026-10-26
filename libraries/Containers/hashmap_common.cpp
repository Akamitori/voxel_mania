#include "hashmap_common.h"

HASHMAP_IMPLEMENTATION(cstr, uint32_t)
HASHMAP_IMPLEMENTATION(cstr, size_t)


static int cstr_uint32_t_Hashmap_key_eq(cstr key_a, cstr key_b) {
    return  string_cmp(key_a,key_b);
}

//TODO fix this later
static uint64_t cstr_uint32_t_Hashmap_key_hash(cstr key_a) {
    const uint8_t *byte_ptr = (const uint8_t *) key_a;
    return fnv1a(byte_ptr, strlen(key_a));
}

static int cstr_size_t_Hashmap_key_eq(cstr key_a, cstr key_b) {
    return string_cmp(key_a,key_b);
}

//TODO fix this later
static uint64_t cstr_size_t_Hashmap_key_hash(cstr key_a) {
    const uint8_t *byte_ptr = (const uint8_t *) key_a;
    return fnv1a(byte_ptr, strlen(key_a));
}