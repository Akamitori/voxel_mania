#include "hashmap_common.h"

HASHMAP_IMPLEMENTATION(cstr, uint32_t)
HASHMAP_IMPLEMENTATION(cstr, size_t)


static int cstr_uint32_t_Hashmap_key_eq(const cstr key_a, const cstr key_b) {
    return  string_cmp(key_a,key_b);
}

// I know that both hashes are similar but i would rather double type some code for know
// in the future we can instead assume that the input to both functions is a pointer to bytes and byte count to simplify things
static uint64_t cstr_uint32_t_Hashmap_key_hash(const cstr key_a) {
    const uint8_t *byte_ptr = (const uint8_t *) key_a;
    return fnv1a(byte_ptr, strlen(key_a));
}

static int cstr_size_t_Hashmap_key_eq(const cstr key_a, const cstr key_b) {
    return string_cmp(key_a,key_b);
}

// I know that both hashes are similar but i would rather double type some code for know
// in the future we can instead assume that the input to both functions is a pointer to bytes and byte count to simplify things
static uint64_t cstr_size_t_Hashmap_key_hash(const cstr key_a) {
    const uint8_t *byte_ptr = (const uint8_t *) key_a;
    return fnv1a(byte_ptr, strlen(key_a));
}