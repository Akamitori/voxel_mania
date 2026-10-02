//
// Created by PETROS on 23/01/2026.
//

#ifndef VOXEL_MANIA_STACK_H
#define VOXEL_MANIA_STACK_H

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

#define STACK_DECLARATION_LIBRARY(T)                                       \
typedef struct STACK_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} STACK_##T;                                                               \
                                                                            \
EXPORTED bool STACK_##T##_Push(STACK_##T*, T);                             \
EXPORTED bool STACK_##T##_Pop(STACK_##T*, T*);                             \
EXPORTED bool STACK_##T##_Peek(const STACK_##T*, T*);                        \
EXPORTED STACK_##T* STACK_##T##_Create(int);                              \
EXPORTED void STACK_##T##_Clear(STACK_##T*);                              \
EXPORTED void STACK_##T##_Free(STACK_##T*);                               \
EXPORTED int STACK_##T##_Length(const STACK_##T*);                              \
EXPORTED int STACK_##T##_Capacity(const STACK_##T*);                            \
EXPORTED T* STACK_##T##_Data(STACK_##T*);

#define STACK_DECLARATION_LIBRARY_STATIC(T)                                       \
typedef struct STACK_##T {                                                 \
T* data;                                                               \
T* end;                                                                 \
T* cap_end;                                                             \
} STACK_##T;                                                               \
\
bool STACK_##T##_Push(STACK_##T*, T);                             \
bool STACK_##T##_Pop(STACK_##T*, T*);                             \
bool STACK_##T##_Peek(const STACK_##T*, T*);                        \
STACK_##T* STACK_##T##_Create(int);                              \
void STACK_##T##_Clear(STACK_##T*);                              \
void STACK_##T##_Free(STACK_##T*);                               \
int STACK_##T##_Length(const STACK_##T*);                              \
int STACK_##T##_Capacity(const STACK_##T*);                            \
T* STACK_##T##_Data(STACK_##T*);

#define STACK_IMPLEMENTATION(T)                                            \
static bool STACK_##T##_Resize(STACK_##T *v) {                            \
    size_t old_size = v->end - v->data;                                    \
    size_t old_cap = v->cap_end - v->data;                                 \
    size_t new_cap = old_cap * 2;                                           \
    T *new_data = (T *)realloc(v->data, new_cap * sizeof(T));              \
    if (!new_data) {return false;}                                            \
    memset(new_data+old_cap,0, (new_cap-old_cap)*sizeof(T));               \
    v->data = new_data;                                                    \
    v->end = new_data + old_size;                                           \
    v->cap_end = new_data + new_cap;                                        \
    return true;                                                            \
}                                                                           \
                                                                            \
bool STACK_##T##_Push(STACK_##T *v, T value) {                             \
    if (v->end == v->cap_end && !STACK_##T##_Resize(v)) {                  \
        return false;                                                          \
    }                                                                        \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
bool STACK_##T##_Pop(STACK_##T *v, T* value) {                             \
    if ( STACK_##T##_Length(v) ==0){                                           \
    return false;                                                              \
    }                                                                       \
    *value=*(--v->end);                                                         \
    return true;                                                            \
}                                                                           \
                                                                             \
bool STACK_##T##_Peek(const STACK_##T *v, T* value) {                             \
    if ( STACK_##T##_Length(v) ==0) return false;                         \
    *value=*(v->end-1);                                                         \
    return true;                                                                \
}                                                                           \
                                                                            \
STACK_##T* STACK_##T##_Create(int min_capacity) {                         \
    if (min_capacity <= 0) {min_capacity = 8;}                                \
    STACK_##T *v = (STACK_##T *)malloc(sizeof(STACK_##T));               \
    if (!v) {return NULL;}                                                    \
    v->data = (T *)calloc(min_capacity , sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
int STACK_##T##_Length(const STACK_##T *v) {                                    \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
int STACK_##T##_Capacity(const STACK_##T *v) {                                  \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
T* STACK_##T##_Data(STACK_##T *v) {                                       \
    return v->data;                                                        \
}                                                                           \
                                                                            \
void STACK_##T##_Clear(STACK_##T *v) {                                    \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
void STACK_##T##_Free(STACK_##T *v) {                                     \
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

#define STACK_IMPLEMENTATION_STATIC(T)                                     \
typedef struct STACK_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} STACK_##T;                                                               \
                                                                            \
static inline bool STACK_##T##_Resize(STACK_##T *v) {                     \
    size_t old_size = v->end - v->data;                                    \
    size_t old_cap = v->cap_end - v->data;                                 \
    size_t new_cap = old_cap * 2;                                           \
    T *new_data = (T *)realloc(v->data, new_cap * sizeof(T));              \
    if (!new_data) return false;                                            \
    memset(new_data+old_cap,0, (new_cap-old_cap)*sizeof(T));               \
    v->data = new_data;                                                    \
    v->end = new_data + old_size;                                           \
    v->cap_end = new_data + new_cap;                                        \
    return true;                                                            \
}                                                                           \
                                                                            \
static inline bool STACK_##T##_Push(STACK_##T *v, T value) {               \
    if (v->end == v->cap_end && !STACK_##T##_Resize(v)) return false;      \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
static inline STACK_##T* STACK_##T##_Create(int min_capacity) {           \
    if (min_capacity <= 0) min_capacity = 8;                                \
    STACK_##T *v = (STACK_##T *)malloc(sizeof(STACK_##T));               \
    if (!v) return NULL;                                                    \
    v->data = (T *)calloc(min_capacity , sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
static inline bool STACK_##T##_Pop(STACK_##T *v, T* value) {                             \
    if ( STACK_##T##_Length(v) ==0) return false;                              \
    *value=*(--v->end);                                                         \
    return true;                                                            \
}                                                                           \
                                                                                \
static inline  bool STACK_##T##_Peek(const STACK_##T *v, T* value) {              \
    if ( STACK_##T##_Length(v) ==0) return false;                         \
    *value=*(v->end-1);                                                         \
    return true;                                                                \
}                                                                           \
                                                                            \
static inline int STACK_##T##_Length(const STACK_##T *v) {                      \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
static inline int STACK_##T##_Capacity(const STACK_##T *v) {                    \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
static inline void STACK_##T##_Clear(STACK_##T *v) {                      \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
static inline void STACK_##T##_Free(STACK_##T *v) {                       \
    if (!v) return;                                                         \
    free(v->data);                                                         \
    free(v);                                                                \
}

#endif //VOXEL_MANIA_STACK_H
