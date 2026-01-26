//
// Created by PETROS on 23/01/2026.
//

#ifndef VOXEL_MANIA_VECTOR_H
#define VOXEL_MANIA_VECTOR_H

#include "export.h"
#include <string.h> 
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#define VECTOR_DECLARATION_LIBRARY(T)                           \
typedef struct Vector_##T {                     \
int capacity;                              \
T *data;                                   \
int length;                                 \
} Vector_##T;                                   \
\
EXPORTED bool Vector_##T##_Add(Vector_##T*, T);        \
EXPORTED Vector_##T *Vector_##T##_Create(int);             \
EXPORTED void Vector_##T##_Clear(Vector_##T*);             \
EXPORTED void Vector_##T##_Free(Vector_##T*);
    
#ifdef __cplusplus
    }
#endif

#ifdef __cplusplus
extern "C" {
#endif
#define VECTOR_DECLARATION_STATIC(T)                           \
typedef struct Vector_##T {                     \
int capacity;                              \
T *data;                                   \
int length;                                 \
} Vector_##T;                                   \
\
bool Vector_##T##_Add(Vector_##T*, T);        \
Vector_##T *Vector_##T##_Create(int);             \
void Vector_##T##_Clear(Vector_##T*);             \
void Vector_##T##_Free(Vector_##T*);
    
#ifdef __cplusplus
}
#endif


#define VECTOR_IMPLEMENTATION(T)                                            \
static bool Vector_##T##_Resize(Vector_##T *v) {                            \
    int new_capacity = v->capacity * 2;                                     \
    T *new_data = (T *)calloc(new_capacity, sizeof(T));                     \
                                                                            \
    if (!new_data) return false;                                            \
                                                                            \
    memcpy(new_data, v->data, v->length * sizeof(T));                       \
                                                                            \
    free(v->data);                                                          \
    v->data = new_data;                                                     \
    v->capacity = new_capacity;                                             \
    return true;                                                            \
}                                                                           \
                                                                            \
bool Vector_##T##_Add(Vector_##T *v, T value) {                             \
    if (v->length == v->capacity && !Vector_##T##_Resize(v)) return false;  \
    v->data[v->length] = value;                                             \
    ++v->length;                                                            \
    return true;                                                            \
}                                                                           \
                                                                            \
Vector_##T *Vector_##T##_Create(int min_capacity) {                         \
    if (min_capacity <= 0) min_capacity = 8; /* reasonable default */       \
                                                                            \
    Vector_##T *v = (Vector_##T *)malloc(sizeof(Vector_##T));               \
    if (!v) return NULL;                                                    \
                                                                            \
    /* Next power of 2 >= min_capacity */                                   \
    unsigned int cap = (unsigned int)min_capacity - 1;                      \
    cap |= cap >> 1;                                                        \
    cap |= cap >> 2;                                                        \
    cap |= cap >> 4;                                                        \
    cap |= cap >> 8;                                                        \
    cap |= cap >> 16;                                                       \
    ++cap;                                                                  \
                                                                            \
    v->capacity = cap;                                                      \
    v->data =(T *) calloc(cap, sizeof(T));                                  \
    if (!v->data) {                                                         \
        free(v);                                                            \
        return NULL;                                                        \
    }                                                                       \
    v->length = 0;                                                          \
    return v;                                                               \
}                                                                           \
                                                                            \
void Vector_##T##_Clear(Vector_##T *pv) {                                   \
pv->length=0;                                                               \
}                                                                           \
void Vector_##T##_Free(Vector_##T *pv) {                                    \
    free(pv->data);                                                         \
    free(pv);                                                               \
}                                

#endif //VOXEL_MANIA_VECTOR_H