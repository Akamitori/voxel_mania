#ifndef VOXEL_MANIA_HASHMAP_COMMON_H
#define VOXEL_MANIA_HASHMAP_COMMON_H


#include "hashmap_container.h"
#include "common_hashmaps_export.h"

typedef const char *cstr;
HASHMAP_DECLARATION_LIBRARY(cstr,uint32_t,COMMON_HASHMAPS_EXPORT)
HASHMAP_DECLARATION_LIBRARY(cstr,size_t,COMMON_HASHMAPS_EXPORT)          \

inline int string_cmp(const cstr key_a, const cstr key_b) {
    return strcmp(key_a, key_b) == 0;
}



#endif //VOXEL_MANIA_HASHMAP_COMMON_H