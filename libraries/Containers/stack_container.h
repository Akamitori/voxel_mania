//
// Created by PETROS on 23/01/2026.
//

#ifndef VOXEL_MANIA_STACK_H
#define VOXEL_MANIA_STACK_H

#include "export.h"
#include <string.h>
#include <stdlib.h>

#ifndef __cplusplus
// include c boolean header
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================================================
// Library version (exported, for use across translation units)
// ==========================================================================

#define STACK_DECLARATION_LIBRARY(T)                                       \
typedef struct Stack_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} Stack_##T;                                                               \
                                                                            \
EXPORTED bool Stack_##T##_Push(Stack_##T*, T);                             \
EXPORTED bool Stack_##T##_Pop(Stack_##T*, T*);                             \
EXPORTED bool Stack_##T##_Peek(const Stack_##T*, T*);                        \
EXPORTED Stack_##T* Stack_##T##_Create(int);                              \
EXPORTED void Stack_##T##_Clear(Stack_##T*);                              \
EXPORTED void Stack_##T##_Free(Stack_##T*);                               \
EXPORTED int Stack_##T##_Length(const Stack_##T*);                              \
EXPORTED int Stack_##T##_Capacity(const Stack_##T*);                            \
EXPORTED T* Stack_##T##_Data(Stack_##T*);

// ==========================================================================
// Internal version (for declaring the function in the same c/cpp file for internal use . PS. this is probably an overkill sicne usually we define the static inline version on top anyways
// ==========================================================================    
    
#define STACK_DECLARATION_LIBRARY_STATIC(T)                                       \
typedef struct Stack_##T {                                                 \
T* data;                                                               \
T* end;                                                                 \
T* cap_end;                                                             \
} Stack_##T;                                                               \
\
bool Stack_##T##_Push(Stack_##T*, T);                             \
bool Stack_##T##_Pop(Stack_##T*, T*);                             \
bool Stack_##T##_Peek(const Stack_##T*, T*);                        \
Stack_##T* Stack_##T##_Create(int);                              \
void Stack_##T##_Clear(Stack_##T*);                              \
void Stack_##T##_Free(Stack_##T*);                               \
int Stack_##T##_Length(const Stack_##T*);                              \
int Stack_##T##_Capacity(const Stack_##T*);                            \
T* Stack_##T##_Data(Stack_##T*);

// ==========================================================================
// Implementation of the stack module when it is declare as a library
// ========================================================================== 
    
    
#define STACK_IMPLEMENTATION(T)                                            \
static bool Stack_##T##_Resize(Stack_##T *v) {                            \
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
bool Stack_##T##_Push(Stack_##T *v, T value) {                             \
    if (v->end == v->cap_end && !Stack_##T##_Resize(v)) {                  \
        return false;                                                          \
    }                                                                        \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
bool Stack_##T##_Pop(Stack_##T *v, T* value) {                             \
    if ( Stack_##T##_Length(v) ==0){                                           \
    return false;                                                              \
    }                                                                       \
    *value=*(--v->end);                                                         \
    return true;                                                            \
}                                                                           \
                                                                             \
bool Stack_##T##_Peek(const Stack_##T *v, T* value) {                             \
    if ( Stack_##T##_Length(v) ==0) return false;                         \
    *value=*(v->end-1);                                                         \
    return true;                                                                \
}                                                                           \
                                                                            \
Stack_##T* Stack_##T##_Create(int min_capacity) {                         \
    if (min_capacity <= 0) {min_capacity = 8;}                                \
    Stack_##T *v = (Stack_##T *)malloc(sizeof(Stack_##T));               \
    if (!v) {return NULL;}                                                    \
    v->data = (T *)calloc(min_capacity , sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
int Stack_##T##_Length(const Stack_##T *v) {                                    \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
int Stack_##T##_Capacity(const Stack_##T *v) {                                  \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
T* Stack_##T##_Data(Stack_##T *v) {                                       \
    return v->data;                                                        \
}                                                                           \
                                                                            \
void Stack_##T##_Clear(Stack_##T *v) {                                    \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
void Stack_##T##_Free(Stack_##T *v) {                                     \
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
typedef struct Stack_##T {                                                 \
    T* data;                                                               \
    T* end;                                                                 \
    T* cap_end;                                                             \
} Stack_##T;                                                               \
                                                                            \
static inline int Stack_##T##_Length(const Stack_##T *v);\
                                                          \
static inline bool Stack_##T##_Resize(Stack_##T *v) {                     \
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
static inline bool Stack_##T##_Push(Stack_##T *v, const T value) {               \
    if (v->end == v->cap_end && !Stack_##T##_Resize(v)) return false;      \
    *v->end++ = value;                                                      \
    return true;                                                            \
}                                                                           \
                                                                            \
static inline Stack_##T* Stack_##T##_Create(int min_capacity) {           \
    if (min_capacity <= 0) min_capacity = 8;                                \
    Stack_##T *v = (Stack_##T *)malloc(sizeof(Stack_##T));               \
    if (!v) return NULL;                                                    \
    v->data = (T *)calloc(min_capacity , sizeof(T));                       \
    if (!v->data) { free(v); return NULL; }                                \
    v->end = v->data;                                                      \
    v->cap_end = v->data + min_capacity;                                   \
    return v;                                                               \
}                                                                           \
                                                                            \
static inline bool Stack_##T##_Pop(Stack_##T *v, T* value) {                             \
    if ( Stack_##T##_Length(v) ==0) return false;                              \
    *value=*(--v->end);                                                         \
    return true;                                                            \
}                                                                           \
                                                                                \
static inline  bool Stack_##T##_Peek(const Stack_##T *v, T* value) {              \
    if ( Stack_##T##_Length(v) ==0) return false;                         \
    *value=*(v->end-1);                                                         \
    return true;                                                                \
}                                                                           \
                                                                            \
static inline int Stack_##T##_Length(const Stack_##T *v) {                      \
    return (int)(v->end - v->data);                                        \
}                                                                           \
                                                                            \
static inline int Stack_##T##_Capacity(const Stack_##T *v) {                    \
    return (int)(v->cap_end - v->data);                                    \
}                                                                           \
                                                                            \
static inline void Stack_##T##_Clear(Stack_##T *v) {                      \
    v->end = v->data;                                                      \
}                                                                           \
                                                                            \
static inline void Stack_##T##_Free(Stack_##T *v) {                       \
    if (!v) return;                                                         \
    free(v->data);                                                         \
    free(v);                                                                \
}

#endif //VOXEL_MANIA_STACK_H
