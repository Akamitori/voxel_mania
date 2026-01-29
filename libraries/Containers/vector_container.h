//
// Created by PETROS on 23/01/2026.
//

#ifndef VOXEL_MANIA_VECTOR_H
#define VOXEL_MANIA_VECTOR_H

#include "export.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================================================
// Library version (exported, for use across translation units)
// ==========================================================================

#define VECTOR_DECLARATION_LIBRARY(T)                                       \
typedef struct Vector_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} Vector_##T;                                                               \
                                                                            \
EXPORTED bool Vector_##T##_Add(Vector_##T*, T);                             \
EXPORTED Vector_##T* Vector_##T##_Create(int);                              \
EXPORTED void Vector_##T##_Clear(Vector_##T*);                              \
EXPORTED void Vector_##T##_Free(Vector_##T*);                               \
EXPORTED int Vector_##T##_Length(Vector_##T*);                              \
EXPORTED int Vector_##T##_Capacity(Vector_##T*);                            \
EXPORTED T* Vector_##T##_Data(Vector_##T*);
    
#define VECTOR_DECLARATION_LIBRARY_STATIC(T)                                       \
typedef struct Vector_##T {                                                 \
T* data;                                                               \
T* end;                                                                 \
T* cap_end;                                                             \
} Vector_##T;                                                               \
\
bool Vector_##T##_Add(Vector_##T*, T);                             \
Vector_##T* Vector_##T##_Create(int);                              \
void Vector_##T##_Clear(Vector_##T*);                              \
void Vector_##T##_Free(Vector_##T*);                               \
int Vector_##T##_Length(Vector_##T*);                              \
int Vector_##T##_Capacity(Vector_##T*);                            \
T* Vector_##T##_Data(Vector_##T*);

#define VECTOR_IMPLEMENTATION(T)                                            \
static bool Vector_##T##_Resize(Vector_##T *v) {                            \
    size_t old_size = v->end - v->data;                                    \
    size_t old_cap = v->cap_end - v->data;                                 \
    size_t new_cap = old_cap * 2;                                           \
    T *new_data = (T *)realloc(v->data, new_cap * sizeof(T));              \
    if (!new_data) return false;                                            \
    v->data = new_data;                                                    \
    v->end = new_data + old_size;                                           \
    v->cap_end = new_data + new_cap;                                        \
    return true;                                                            \
}                                                                           \
                                                                            \
bool Vector_##T##_Add(Vector_##T *v, T value) {                             \
    if (v->end == v->cap_end && !Vector_##T##_Resize(v)) return false;      \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
Vector_##T* Vector_##T##_Create(int min_capacity) {                         \
    if (min_capacity <= 0) min_capacity = 8;                                \
    Vector_##T *v = (Vector_##T *)malloc(sizeof(Vector_##T));               \
    if (!v) return NULL;                                                    \
    v->data = (T *)malloc(min_capacity * sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
int Vector_##T##_Length(Vector_##T *v) {                                    \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
int Vector_##T##_Capacity(Vector_##T *v) {                                  \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
T* Vector_##T##_Data(Vector_##T *v) {                                       \
    return v->data;                                                        \
}                                                                           \
                                                                            \
void Vector_##T##_Clear(Vector_##T *v) {                                    \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
void Vector_##T##_Free(Vector_##T *v) {                                     \
    if (!v) return;                                                         \
    free(v->data);                                                         \
    free(v);                                                                \
}

#ifdef __cplusplus
}
#endif

// ==========================================================================
// Static inline version (header-only, for maximum performance)
// ==========================================================================

#define VECTOR_IMPLEMENTATION_STATIC(T)                                     \
typedef struct Vector_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} Vector_##T;                                                               \
                                                                            \
static inline bool Vector_##T##_Resize(Vector_##T *v) {                     \
    size_t old_size = v->end - v->data;                                    \
    size_t old_cap = v->cap_end - v->data;                                 \
    size_t new_cap = old_cap * 2;                                           \
    T *new_data = (T *)realloc(v->data, new_cap * sizeof(T));              \
    if (!new_data) return false;                                            \
    v->data = new_data;                                                    \
    v->end = new_data + old_size;                                           \
    v->cap_end = new_data + new_cap;                                        \
    return true;                                                            \
}                                                                           \
                                                                            \
static inline bool Vector_##T##_Add(Vector_##T *v, T value) {               \
    if (v->end == v->cap_end && !Vector_##T##_Resize(v)) return false;      \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
static inline Vector_##T* Vector_##T##_Create(int min_capacity) {           \
    if (min_capacity <= 0) min_capacity = 8;                                \
    Vector_##T *v = (Vector_##T *)malloc(sizeof(Vector_##T));               \
    if (!v) return NULL;                                                    \
    v->data = (T *)malloc(min_capacity * sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
static inline int Vector_##T##_Length(Vector_##T *v) {                      \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
static inline int Vector_##T##_Capacity(Vector_##T *v) {                    \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
static inline void Vector_##T##_Clear(Vector_##T *v) {                      \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
static inline void Vector_##T##_Free(Vector_##T *v) {                       \
    if (!v) return;                                                         \
    free(v->data);                                                         \
    free(v);                                                                \
}

#endif //VOXEL_MANIA_VECTOR_H