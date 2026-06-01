//
// Created by PETROS on 29/09/2026.
//


#ifndef VOXEL_MANIA_HASHMAP_CONTAINER_H
#define VOXEL_MANIA_HASHMAP_CONTAINER_H

#include "export.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


#ifdef __cplusplus
#define C_LINKAGE extern "C"
#else
#define C_LINKAGE
#endif

static inline uint64_t fnv1a(const uint8_t *s, size_t byte_count) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < byte_count; i++) {
        hash ^= s[i];
        hash *= 1099511628211ULL;
    }
    hash ^= hash >> 32;
    return hash;
}



#define HASHMAP_STRUCTS(KEY,VALUE)                                          \
typedef struct  {                                                       \
    KEY key;                                                            \
    VALUE value;                                                           \
} KEY##_##VALUE##_hashmap_pair;                                         \
                                                                          \
                                                                          \
typedef struct  {                                                         \
    KEY##_##VALUE##_hashmap_pair pair;                                            \
    uint16_t PSL;                                                          \
    uint8_t occupied;                                                        \
} KEY##_##VALUE##_hashmap_slot;                                        \
                                                                        \
                                                                       \
                                                                        \
                                                                         \
typedef struct {                                                         \
    KEY##_##VALUE##_hashmap_slot *slots;                             \
    size_t capacity;                                                   \
    size_t num_elements;                                               \
    size_t grow_at;                                                    \
    size_t mask;                                                       \
    float max_load_factor;                                             \
    size_t total_PSL;                                                  \
} KEY##_##VALUE##_hashmap ;                                            \
                                                                        \
                                                                        \
                                                                        \
typedef struct {      \
int found;                                            \
size_t index;                                           \
} KEY##_##VALUE##_Hashmap_Index_Find_Result;

#define HASHMAP_DECLARATION_LIBRARY(KEY,VALUE)                                          \
                                                                                \
HASHMAP_STRUCTS(KEY,VALUE) \
C_LINKAGE int KEY##_##VALUE##_Hashmap_key_eq(KEY key_a, KEY key_b);       \
C_LINKAGE uint64_t KEY##_##VALUE##_Hashmap_key_hash(KEY key_a);     \
                                                                                \
   \
C_LINKAGE EXPORTED KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor); \
C_LINKAGE EXPORTED void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h); \
                                                                            \
                                                           \
C_LINKAGE EXPORTED int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);         \
C_LINKAGE EXPORTED VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key);        \
C_LINKAGE EXPORTED void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key);


#define HASHMAP_IMPLEMENTATION_STATIC(KEY,VALUE)                                                                                                \
HASHMAP_STRUCTS(KEY,VALUE)                                                                                                                       \
static int KEY##_##VALUE##_Hashmap_key_eq(KEY key_a, KEY key_b);                                                                                  \
static uint64_t KEY##_##VALUE##_Hashmap_key_hash(KEY key_a);                                                                                           \
\
\
static KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor); \
static void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h); \
\
\
static int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);         \
static VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key);        \
static void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key);                    \
                                                                                                                                               \
static void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);          \
static KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key);\
\
static KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor) {   \
if (min_capacity <= 0) min_capacity = 8; /* reasonable default */                    \
                                                                                     \
KEY##_##VALUE##_hashmap *h = (KEY##_##VALUE##_hashmap *) malloc(sizeof(KEY##_##VALUE##_hashmap));            \
if (!h) return NULL;                                                                 \
                                                                                     \
/* Next power of 2 >= min_capacity */                                                \
unsigned int cap = (unsigned int) min_capacity - 1;                                  \
cap |= cap >> 1;                                                                     \
cap |= cap >> 2;                                                                     \
cap |= cap >> 4;                                                                     \
cap |= cap >> 8;                                                                     \
cap |= cap >> 16;                                                                    \
++cap;                                                                               \
h->capacity = cap;                                                                   \
h->num_elements = 0;                                                                 \
h->total_PSL = 0;                                                                    \
                                                                                    \
h->slots = (KEY##_##VALUE##_hashmap_slot *) calloc(cap, sizeof(KEY##_##VALUE##_hashmap_slot));       \
h->grow_at = (size_t) (cap * max_load_factor);                                       \
h->max_load_factor = max_load_factor;                                                \
h->mask = cap - 1;                                                                   \
                                                                                     \
if (!h->slots) {                                                                     \
    free(h);                                                                         \
    return NULL;                                                                     \
}                                                                                    \
                                                                                     \
return h;                                                                            \
}                                                                                    \
\
\
\
static void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h) {                                     \
free(h->slots);                                                      \
free(h);                                                              \
}                                                                      \
\
\
\
\
\
static  int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) {                                                   \
    assert(h != 0 && "null pointer wtf bro");                                                                                     \
    VALUE* result=KEY##_##VALUE##_Hashmap_Find(h, pair_to_insert.key);                            \
    if(result) {                        \
    *result=pair_to_insert.value;       \
    return 1;                           \
    }\
                                                                                                                                        \
    if (h->num_elements + 1 > h->grow_at) {                                                                                             \
        const size_t new_cap = h->capacity * 2;                                                                                         \
        KEY##_##VALUE##_hashmap_slot *newData = (KEY##_##VALUE##_hashmap_slot *) calloc(new_cap, sizeof(KEY##_##VALUE##_hashmap_slot));                         \
                                                                                                                                        \
        if (!newData) {                                                                                                                 \
            return 0;                                                                                                                   \
        }                                                                                                                               \
                                                                                                                                        \
        KEY##_##VALUE##_hashmap_slot *oldData = h->slots;                                                                                       \
        const size_t old_cap = h->capacity;                                                                                             \
                                                                                                                                        \
        h->slots = newData;                                                                                                             \
        h->capacity = new_cap;                                                                                                          \
        h->total_PSL = 0;                                                                                                               \
        h->num_elements = 0;                                                                                                            \
        h->grow_at = (size_t) (new_cap * h->max_load_factor);                                                                           \
        h->mask = new_cap - 1;                                                                                                          \
                                                                                                                                        \
        for (size_t i = 0; i < old_cap; ++i) {                                                                                             \
            if (oldData[i].occupied) {                                                                                                  \
                KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, oldData[i].pair);                                                                   \
            }                                                                                                                           \
        }                                                                                                                               \
                                                                                                                                        \
        free(oldData);                                                                                                                  \
    }                                                                                                                                   \
    KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, pair_to_insert);                                                                                \
    return 1;                                                                                                                           \
}                                                                                                                                       \
\
static void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) { \
    size_t current_PSL = 0;                                                                                                            \
    uint64_t hash =KEY##_##VALUE##_Hashmap_key_hash(pair_to_insert.key);                                                             \
                                                                                                                                       \
    size_t i = hash & h->mask;                                                                                                     \
    while (1) {                                                                                                                     \
        KEY##_##VALUE##_hashmap_slot *slot = &h->slots[i];                                                                           \
                                                                                                                                       \
        if (!slot->occupied) {                                                                                                         \
            slot->pair = pair_to_insert;                                                                                               \
            slot->occupied = 1;                                                                                                     \
            slot->PSL = current_PSL;                                                                                                   \
            h->total_PSL += current_PSL;                                                                                               \
            h->num_elements++;                                                                                                         \
            assert(current_PSL <= UINT16_MAX && "we oveflowed");           \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
        if (KEY##_##VALUE##_Hashmap_key_eq(slot->pair.key,pair_to_insert.key)) {                                                                                    \
            slot->pair.value = pair_to_insert.value;                                                                                   \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
                                                                                                                                       \
        if (slot->PSL < current_PSL) {                                                                                                 \
            const KEY##_##VALUE##_hashmap_pair tmp_pair = slot->pair;                                                                \
            const size_t tmp_PSL = slot->PSL;                                                                                          \
                                                                                                                                       \
            slot->pair = pair_to_insert;                                                                                               \
            slot->PSL = current_PSL;                                                                                                   \
                                                                                                                                       \
            h->total_PSL += current_PSL - tmp_PSL;                                                                                     \
                                                                                                                                       \
            current_PSL = tmp_PSL;                                                                                                     \
            pair_to_insert = tmp_pair;                                                                                                 \
        }                                                                                                                              \
        i = (i + 1) & h->mask;                                                                                                         \
        current_PSL++;                                                                                                                 \
    }                                                                                                                                  \
}                                                                                                                                       \
                                                                                                                                        \
static VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key) {                   \
KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                             \
                                                                                                                                         \
if (!result.found) {                                                                                                                      \
    return NULL;                                                                                                                        \
}                                                                                                                                           \
                                                                                                                                             \
return &h->slots[result.index].pair.value;                                                                                                          \
}                                                                                                                                              \
                                                                                                                                         \
static KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key) {   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result;                   \
    result.found=0;           \
    result.index=0;       \
    if (h->num_elements == 0) {                                                                                                                     \
        return result;             \
    }                                                                                                                                               \
                                                                                                                                                    \
    uint64_t hash = KEY##_##VALUE##_Hashmap_key_hash(key);                                                                                        \
                                                                                                                                                    \
    const size_t calculated_index_slot = hash & h->mask;                                                                                            \
                                                                                                                                                    \
    const size_t meanPSL = h->total_PSL / h->num_elements;                                                                                          \
                                                                                                                                                    \
    const size_t i = calculated_index_slot;                                                                                                         \
    for (size_t step = meanPSL; step-- > 0;) {                                                                                                      \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            continue;                                                                                                                               \
        }                                                                                                                                           \
                                                                                                                                                    \
        if (step < current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
                                                                                                                                                    \
    for (size_t step = meanPSL; ; ++step) {                                                                                                         \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
                                                                                                                                                     \
                                                                                                                                                    \
        if (step > current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
    return result;                                                                                                                                  \
}                                                                                                                                                   \
                                                                                                                                                    \
static void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key) {                                                                   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                                    \
                                                                                                                                                   \
    if (!result.found) {                                                                                                                           \
        return;                                                                                                                                    \
    }                                                                                                                                              \
                                                                                                                                                   \
    size_t current_slot_index = result.index;                                                                                                      \
                                                                                                                                                   \
                                                                                                                                                    \
    KEY##_##VALUE##_hashmap_slot *empty_slot = &h->slots[current_slot_index];                                                                    \
    h->total_PSL -= empty_slot->PSL;                                                                                                               \
    empty_slot->occupied = 0;                                                                                                                  \
    h->num_elements-=1;                                                                                                                            \
                                                                                                                                                   \
    while (1) {                                                                                                                                 \
        size_t next_slot_index = (current_slot_index + 1) & h->mask;                                                                               \
        KEY##_##VALUE##_hashmap_slot *slot_to_move = &h->slots[next_slot_index];                                                                 \
                                                                                                                                                   \
        if (!slot_to_move->occupied || slot_to_move->PSL == 0) {                                                                                   \
            break;                                                                                                                                 \
        }                                                                                                                                          \
                                                                                                                                                   \
        *empty_slot = *slot_to_move;                                                                                                               \
        empty_slot->PSL -= 1;                                                                                                                      \
        h->total_PSL -= 1;                                                                                                                         \
                                                                                                                                                   \
        current_slot_index = next_slot_index;                                                                                                      \
        empty_slot = &h->slots[current_slot_index];                                                                                                \
        empty_slot->occupied = 0;                                                                                                              \
    }                                                                                                                                              \
}



#define HASHMAP_IMPLEMENTATION(KEY,VALUE)  \
                                                                                                                                           \
                                                                                                                                         \
                                                                                                                                        \
static void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);          \
static KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key);\
KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor) {   \
if (min_capacity <= 0) min_capacity = 8; /* reasonable default */                    \
                                                                                     \
KEY##_##VALUE##_hashmap *h = (KEY##_##VALUE##_hashmap *) malloc(sizeof(KEY##_##VALUE##_hashmap));            \
if (!h) return NULL;                                                                 \
                                                                                     \
/* Next power of 2 >= min_capacity */                                                \
unsigned int cap = (unsigned int) min_capacity - 1;                                  \
cap |= cap >> 1;                                                                     \
cap |= cap >> 2;                                                                     \
cap |= cap >> 4;                                                                     \
cap |= cap >> 8;                                                                     \
cap |= cap >> 16;                                                                    \
++cap;                                                                               \
h->capacity = cap;                                                                   \
h->num_elements = 0;                                                                 \
h->total_PSL = 0;                                                                    \
                                                                                    \
h->slots = (KEY##_##VALUE##_hashmap_slot *) calloc(cap, sizeof(KEY##_##VALUE##_hashmap_slot));       \
h->grow_at = (size_t) (cap * max_load_factor);                                       \
h->max_load_factor = max_load_factor;                                                \
h->mask = cap - 1;                                                                   \
                                                                                     \
if (!h->slots) {                                                                     \
    free(h);                                                                         \
    return NULL;                                                                     \
}                                                                                    \
                                                                                     \
return h;                                                                            \
}                                                                                    \
\
\
\
void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h) {                                     \
free(h->slots);                                                      \
free(h);                                                              \
}                                                                      \
\
\
\
\
\
int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) {                                                   \
    assert(h != 0 && "null pointer wtf bro");                                                                                     \
    VALUE* result=KEY##_##VALUE##_Hashmap_Find(h, pair_to_insert.key);                            \
     if(result) {                        \
        *result=pair_to_insert.value;       \
        return 1;                           \
     }\
                                                                                                                                        \
    if (h->num_elements + 1 > h->grow_at) {                                                                                             \
        const size_t new_cap = h->capacity * 2;                                                                                         \
        KEY##_##VALUE##_hashmap_slot *newData = (KEY##_##VALUE##_hashmap_slot *) calloc(new_cap, sizeof(KEY##_##VALUE##_hashmap_slot));                         \
                                                                                                                                        \
        if (!newData) {                                                                                                                 \
            return 0;                                                                                                                   \
        }                                                                                                                               \
                                                                                                                                        \
        KEY##_##VALUE##_hashmap_slot *oldData = h->slots;                                                                                       \
        const size_t old_cap = h->capacity;                                                                                             \
                                                                                                                                        \
        h->slots = newData;                                                                                                             \
        h->capacity = new_cap;                                                                                                          \
        h->total_PSL = 0;                                                                                                               \
        h->num_elements = 0;                                                                                                            \
        h->grow_at = (size_t) (new_cap * h->max_load_factor);                                                                           \
        h->mask = new_cap - 1;                                                                                                          \
                                                                                                                                        \
        for (size_t i = 0; i < old_cap; ++i) {                                                                                             \
            if (oldData[i].occupied) {                                                                                                  \
                KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, oldData[i].pair);                                                                   \
            }                                                                                                                           \
        }                                                                                                                               \
                                                                                                                                        \
        free(oldData);                                                                                                                  \
    }                                                                                                                                   \
    KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, pair_to_insert);                                                                                \
    return 1;                                                                                                                           \
}                                                                                                                                       \
\
static void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) { \
    size_t current_PSL = 0;                                                                                                            \
    uint64_t hash =KEY##_##VALUE##_Hashmap_key_hash(pair_to_insert.key);                                                             \
                                                                                                                                       \
    size_t i = hash & h->mask;                                                                                                     \
    while (1) {                                                                                                                     \
        KEY##_##VALUE##_hashmap_slot *slot = &h->slots[i];                                                                           \
                                                                                                                                       \
        if (!slot->occupied) {                                                                                                         \
            slot->pair = pair_to_insert;                                                                                               \
            slot->occupied = 1;                                                                                                     \
            slot->PSL = current_PSL;                                                                                                   \
            h->total_PSL += current_PSL;                                                                                               \
            h->num_elements++;                          \
            assert(current_PSL <= UINT16_MAX && "we oveflowed");           \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
        if (KEY##_##VALUE##_Hashmap_key_eq(slot->pair.key,pair_to_insert.key)) {                                                                                    \
            slot->pair.value = pair_to_insert.value;                                                                                   \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
                                                                                                                                       \
        if (slot->PSL < current_PSL) {                                                                                                 \
            const KEY##_##VALUE##_hashmap_pair tmp_pair = slot->pair;                                                                \
            const size_t tmp_PSL = slot->PSL;                                                                                          \
                                                                                                                                       \
            slot->pair = pair_to_insert;                                                                                               \
            slot->PSL = current_PSL;                                                                                                   \
                                                                                                                                       \
            h->total_PSL += current_PSL - tmp_PSL;                                                                                     \
                                                                                                                                       \
            current_PSL = tmp_PSL;                                                                                                     \
            pair_to_insert = tmp_pair;                                                                                                 \
        }                                                                                                                              \
        i = (i + 1) & h->mask;                                                                                                         \
        current_PSL++;                                                                                                                 \
    }                                                                                                                                  \
}                                                                                                                                       \
                                                                                                                                        \
VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key) {                   \
KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                             \
                                                                                                                                         \
if (!result.found) {                                                                                                                      \
    return NULL;                                                                                                                        \
}                                                                                                                                           \
                                                                                                                                             \
return &h->slots[result.index].pair.value;                                                                                                          \
}                                                                                                                                              \
                                                                                                                                         \
static KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key) {   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result;            \
    result.found=0;                                                                                                                         \
    result.index=0;                                                                                                     \
    if (h->num_elements == 0) {                                                                                                       \
        return result;                                                                                                                              \
    }                                                                                                                                               \
                                                                                                                                                    \
    uint64_t hash = KEY##_##VALUE##_Hashmap_key_hash(key);                                                                                        \
                                                                                                                                                    \
    const size_t calculated_index_slot = hash & h->mask;                                                                                            \
                                                                                                                                                    \
    const size_t meanPSL = h->total_PSL / h->num_elements;                                                                                          \
                                                                                                                                                    \
    const size_t i = calculated_index_slot;                                                                                                         \
    for (size_t step = meanPSL; step-- > 0;) {                                                                                                      \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            continue;                                                                                                                               \
        }                                                                                                                                           \
                                                                                                                                                    \
        if (step < current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
                                                                                                                                                    \
    for (size_t step = meanPSL; ; ++step) {                                                                                                         \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        if (step > current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
    return result;                                                                                                                                 \
}                                                                                                                                                   \
                                                                                                                                                    \
void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key) {                                                                   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                                    \
                                                                                                                                                   \
    if (!result.found) {                                                                                                                           \
        return;                                                                                                                                    \
    }                                                                                                                                              \
                                                                                                                                                   \
    size_t current_slot_index = result.index;                                                                                                      \
                                                                                                                                                   \
                                                                                                                                                    \
    KEY##_##VALUE##_hashmap_slot *empty_slot = &h->slots[current_slot_index];                                                                    \
    h->total_PSL -= empty_slot->PSL;                                                                                                               \
    empty_slot->occupied = 0;                                                                                                                  \
    h->num_elements-=1;                                                                                                                            \
                                                                                                                                                   \
    while (1) {                                                                                                                                 \
        size_t next_slot_index = (current_slot_index + 1) & h->mask;                                                                               \
        KEY##_##VALUE##_hashmap_slot *slot_to_move = &h->slots[next_slot_index];                                                                 \
                                                                                                                                                   \
        if (!slot_to_move->occupied || slot_to_move->PSL == 0) {                                                                                   \
            break;                                                                                                                                 \
        }                                                                                                                                          \
                                                                                                                                                   \
        *empty_slot = *slot_to_move;                                                                                                               \
        empty_slot->PSL -= 1;                                                                                                                      \
        h->total_PSL -= 1;                                                                                                                         \
                                                                                                                                                   \
        current_slot_index = next_slot_index;                                                                                                      \
        empty_slot = &h->slots[current_slot_index];                                                                                                \
        empty_slot->occupied = 0;                                                                                                              \
    }                                                                                                                                              \
}


#define HASHMAP_IMPLEMENTATION_HEADER(KEY,VALUE)                                                                                                \
HASHMAP_STRUCTS(KEY,VALUE)                                                                                                                       \
static int KEY##_##VALUE##_Hashmap_key_eq(KEY key_a, KEY key_b);                                                                                  \
static uint64_t KEY##_##VALUE##_Hashmap_key_hash(KEY key_a);                                                                                           \
\
\
KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor); \
void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h); \
\
\
int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);         \
VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key);        \
void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key);                    \
                                                                                                                                               \
void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert);          \
KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key);\
\
KEY##_##VALUE##_hashmap *KEY##_##VALUE##_Hashmap_Create(int min_capacity, float max_load_factor) {   \
if (min_capacity <= 0) min_capacity = 8; /* reasonable default */                    \
                                                                                     \
KEY##_##VALUE##_hashmap *h = (KEY##_##VALUE##_hashmap *) malloc(sizeof(KEY##_##VALUE##_hashmap));            \
if (!h) return NULL;                                                                 \
                                                                                     \
/* Next power of 2 >= min_capacity */                                                \
unsigned int cap = (unsigned int) min_capacity - 1;                                  \
cap |= cap >> 1;                                                                     \
cap |= cap >> 2;                                                                     \
cap |= cap >> 4;                                                                     \
cap |= cap >> 8;                                                                     \
cap |= cap >> 16;                                                                    \
++cap;                                                                               \
h->capacity = cap;                                                                   \
h->num_elements = 0;                                                                 \
h->total_PSL = 0;                                                                    \
                                                                                    \
h->slots = (KEY##_##VALUE##_hashmap_slot *) calloc(cap, sizeof(KEY##_##VALUE##_hashmap_slot));       \
h->grow_at = (size_t) (cap * max_load_factor);                                       \
h->max_load_factor = max_load_factor;                                                \
h->mask = cap - 1;                                                                   \
                                                                                     \
if (!h->slots) {                                                                     \
    free(h);                                                                         \
    return NULL;                                                                     \
}                                                                                    \
                                                                                     \
return h;                                                                            \
}                                                                                    \
\
\
\
void KEY##_##VALUE##_Hashmap_Free(KEY##_##VALUE##_hashmap *h) {                                     \
free(h->slots);                                                      \
free(h);                                                              \
}                                                                      \
\
\
\
\
\
int KEY##_##VALUE##_Hashmap_Insert(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) {                                                   \
    assert(h != 0 && "null pointer wtf bro");                                                                                     \
    VALUE* result=KEY##_##VALUE##_Hashmap_Find(h, pair_to_insert.key);                            \
    if(result) {                        \
    *result=pair_to_insert.value;       \
    return 1;                           \
    }\
                                                                                                                                        \
    if (h->num_elements + 1 > h->grow_at) {                                                                                             \
        const size_t new_cap = h->capacity * 2;                                                                                         \
        KEY##_##VALUE##_hashmap_slot *newData = (KEY##_##VALUE##_hashmap_slot *) calloc(new_cap, sizeof(KEY##_##VALUE##_hashmap_slot));                         \
                                                                                                                                        \
        if (!newData) {                                                                                                                 \
            return 0;                                                                                                                   \
        }                                                                                                                               \
                                                                                                                                        \
        KEY##_##VALUE##_hashmap_slot *oldData = h->slots;                                                                                       \
        const size_t old_cap = h->capacity;                                                                                             \
                                                                                                                                        \
        h->slots = newData;                                                                                                             \
        h->capacity = new_cap;                                                                                                          \
        h->total_PSL = 0;                                                                                                               \
        h->num_elements = 0;                                                                                                            \
        h->grow_at = (size_t) (new_cap * h->max_load_factor);                                                                           \
        h->mask = new_cap - 1;                                                                                                          \
                                                                                                                                        \
        for (size_t i = 0; i < old_cap; ++i) {                                                                                             \
            if (oldData[i].occupied) {                                                                                                  \
                KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, oldData[i].pair);                                                                   \
            }                                                                                                                           \
        }                                                                                                                               \
                                                                                                                                        \
        free(oldData);                                                                                                                  \
    }                                                                                                                                   \
    KEY##_##VALUE##_Hashmap_Insert_To_Bucket(h, pair_to_insert);                                                                                \
    return 1;                                                                                                                           \
}                                                                                                                                       \
\
void KEY##_##VALUE##_Hashmap_Insert_To_Bucket(KEY##_##VALUE##_hashmap *h, KEY##_##VALUE##_hashmap_pair pair_to_insert) { \
    size_t current_PSL = 0;                                                                                                            \
    uint64_t hash =KEY##_##VALUE##_Hashmap_key_hash(pair_to_insert.key);                                                             \
                                                                                                                                       \
    size_t i = hash & h->mask;                                                                                                     \
    while (1) {                                                                                                                     \
        KEY##_##VALUE##_hashmap_slot *slot = &h->slots[i];                                                                           \
                                                                                                                                       \
        if (!slot->occupied) {                                                                                                         \
            slot->pair = pair_to_insert;                                                                                               \
            slot->occupied = 1;                                                                                                     \
            slot->PSL = current_PSL;                                                                                                   \
            h->total_PSL += current_PSL;                                                                                               \
            h->num_elements++;                                                                                                         \
            assert(current_PSL <= UINT16_MAX && "we oveflowed");           \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
        if (KEY##_##VALUE##_Hashmap_key_eq(slot->pair.key,pair_to_insert.key)) {                                                                                    \
            slot->pair.value = pair_to_insert.value;                                                                                   \
            return;                                                                                                                    \
        }                                                                                                                              \
                                                                                                                                       \
                                                                                                                                       \
        if (slot->PSL < current_PSL) {                                                                                                 \
            const KEY##_##VALUE##_hashmap_pair tmp_pair = slot->pair;                                                                \
            const size_t tmp_PSL = slot->PSL;                                                                                          \
                                                                                                                                       \
            slot->pair = pair_to_insert;                                                                                               \
            slot->PSL = current_PSL;                                                                                                   \
                                                                                                                                       \
            h->total_PSL += current_PSL - tmp_PSL;                                                                                     \
                                                                                                                                       \
            current_PSL = tmp_PSL;                                                                                                     \
            pair_to_insert = tmp_pair;                                                                                                 \
        }                                                                                                                              \
        i = (i + 1) & h->mask;                                                                                                         \
        current_PSL++;                                                                                                                 \
    }                                                                                                                                  \
}                                                                                                                                       \
                                                                                                                                        \
VALUE *KEY##_##VALUE##_Hashmap_Find(const KEY##_##VALUE##_hashmap *h, const KEY key) {                   \
KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                             \
                                                                                                                                         \
if (!result.found) {                                                                                                                      \
    return NULL;                                                                                                                        \
}                                                                                                                                           \
                                                                                                                                             \
return &h->slots[result.index].pair.value;                                                                                                          \
}                                                                                                                                              \
                                                                                                                                         \
KEY##_##VALUE##_Hashmap_Index_Find_Result KEY##_##VALUE##_Hashmap_Find_Internal_Index(const KEY##_##VALUE##_hashmap *h, const KEY key) {   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result;                   \
    result.found=0;           \
    result.index=0;       \
    if (h->num_elements == 0) {                                                                                                                     \
        return result;             \
    }                                                                                                                                               \
                                                                                                                                                    \
    uint64_t hash = KEY##_##VALUE##_Hashmap_key_hash(key);                                                                                        \
                                                                                                                                                    \
    const size_t calculated_index_slot = hash & h->mask;                                                                                            \
                                                                                                                                                    \
    const size_t meanPSL = h->total_PSL / h->num_elements;                                                                                          \
                                                                                                                                                    \
    const size_t i = calculated_index_slot;                                                                                                         \
    for (size_t step = meanPSL; step-- > 0;) {                                                                                                      \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            continue;                                                                                                                               \
        }                                                                                                                                           \
                                                                                                                                                    \
        if (step < current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
                                                                                                                                                    \
    for (size_t step = meanPSL; ; ++step) {                                                                                                         \
        const size_t index = (i + step) & (h->mask);                                                                                                \
        KEY##_##VALUE##_hashmap_slot *current_slot = &h->slots[index];                                                                            \
                                                                                                                                                    \
        if (!current_slot->occupied) {                                                                                                              \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
                                                                                                                                                     \
                                                                                                                                                    \
        if (step > current_slot->PSL) {                                                                                                             \
            break;                                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        KEY##_##VALUE##_hashmap_pair *current_pair = &current_slot->pair;                                                                         \
        if (KEY##_##VALUE##_Hashmap_key_eq(current_pair->key,key)) {                                                                              \
            result.found=1;           \
            result.index=index;       \
            return result;             \
        }                                                                                                                                           \
    }                                                                                                                                               \
    return result;                                                                                                                                  \
}                                                                                                                                                   \
                                                                                                                                                    \
void KEY##_##VALUE##_Hashmap_Remove(KEY##_##VALUE##_hashmap *h, KEY key) {                                                                   \
    KEY##_##VALUE##_Hashmap_Index_Find_Result result = KEY##_##VALUE##_Hashmap_Find_Internal_Index(h, key);                                    \
                                                                                                                                                   \
    if (!result.found) {                                                                                                                           \
        return;                                                                                                                                    \
    }                                                                                                                                              \
                                                                                                                                                   \
    size_t current_slot_index = result.index;                                                                                                      \
                                                                                                                                                   \
                                                                                                                                                    \
    KEY##_##VALUE##_hashmap_slot *empty_slot = &h->slots[current_slot_index];                                                                    \
    h->total_PSL -= empty_slot->PSL;                                                                                                               \
    empty_slot->occupied = 0;                                                                                                                  \
    h->num_elements-=1;                                                                                                                            \
                                                                                                                                                   \
    while (1) {                                                                                                                                 \
        size_t next_slot_index = (current_slot_index + 1) & h->mask;                                                                               \
        KEY##_##VALUE##_hashmap_slot *slot_to_move = &h->slots[next_slot_index];                                                                 \
                                                                                                                                                   \
        if (!slot_to_move->occupied || slot_to_move->PSL == 0) {                                                                                   \
            break;                                                                                                                                 \
        }                                                                                                                                          \
                                                                                                                                                   \
        *empty_slot = *slot_to_move;                                                                                                               \
        empty_slot->PSL -= 1;                                                                                                                      \
        h->total_PSL -= 1;                                                                                                                         \
                                                                                                                                                   \
        current_slot_index = next_slot_index;                                                                                                      \
        empty_slot = &h->slots[current_slot_index];                                                                                                \
        empty_slot->occupied = 0;                                                                                                              \
    }                                                                                                                                              \
}

#endif //VOXEL_MANIA_HASHMAP_CONTAINER_H