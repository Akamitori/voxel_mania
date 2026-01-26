//
// Created by PETROS on 23/01/2026.
//

#ifndef VOXEL_MANIA_QUEUE_H
#define VOXEL_MANIA_QUEUE_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "export.h"
#include <string.h> 
#include <stdlib.h>

// queue implemented like a ring buffer
// we always use a power of two for size
// this is done so we can then use the mask variable for efficient modulo

#ifdef __cplusplus
    extern "C" {
#endif

#define QUEUE_DECLARATION_LIBRARY(T)                        \
typedef struct Queue_##T {                          \
int capacity;                                       \
T *data;                                            \
int head, tail, size, mask;                         \
} Queue_##T;                                        \
                                                    \
EXPORTED bool Queue_##T##_Enqueue(Queue_##T*, T);   \
EXPORTED bool Queue_##T##_Deque(Queue_##T*, T*);    \
EXPORTED Queue_##T *Queue_##T##_Create(int);        \
EXPORTED void Queue_##T##_Free(Queue_##T*);

#ifdef __cplusplus
    }
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define QUEUE_DECLARATION_STATIC(T)                        \
typedef struct Queue_##T {                          \
int capacity;                                       \
T *data;                                            \
int head, tail, size, mask;                         \
} Queue_##T;                                        \
\
bool Queue_##T##_Enqueue(Queue_##T*, T);   \
bool Queue_##T##_Deque(Queue_##T*, T*);    \
Queue_##T *Queue_##T##_Create(int);        \
void Queue_##T##_Free(Queue_##T*);

#ifdef __cplusplus
}
#endif


#define QUEUE_IMPLEMENTATION(T)                                             \
static bool Queue_##T##_Resize(Queue_##T *q) {                              \
    int new_capacity = q->capacity * 2;                                     \
    T *new_data = (T *)calloc(new_capacity, sizeof(T));                     \
                                                                            \
    if (!new_data) return false;                                            \
                                                                            \
    for (int i = 0; i < q->size; ++i) {                                     \
        int idx = (q->head + i) & q->mask;                                  \
        new_data[i] = q->data[idx];                                         \
    }                                                                       \
                                                                            \
    free(q->data);                                                          \
    q->data = new_data;                                                     \
    q->head = 0;                                                            \
    q->tail = q->size;                                                      \
    q->capacity = new_capacity;                                             \
    q->mask = new_capacity - 1;                                             \
    return true;                                                            \
}                                                                           \
                                                                            \
bool Queue_##T##_Enqueue(Queue_##T *q, T value) {                           \
    if (q->size == q->capacity && !Queue_##T##_Resize(q)) return false;     \
    q->data[q->tail] = value;                                               \
    q->tail = (q->tail + 1) & q->mask;                                      \
    ++q->size;                                                              \
    return true;                                                            \
}                                                                           \
                                                                            \
Queue_##T *Queue_##T##_Create(int min_capacity) {                           \
    if (min_capacity <= 0) min_capacity = 8; /* reasonable default */       \
                                                                            \
    Queue_##T *q = (Queue_##T *)malloc(sizeof(Queue_##T));                               \
    if (!q) return NULL;                                                    \
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
    q->capacity = cap;                                                      \
    q->mask = cap - 1;                                                      \
    q->data =(T *) calloc(cap, sizeof(T));                                       \
    if (!q->data) {                                                         \
        free(q);                                                            \
        return NULL;                                                        \
    }                                                                       \
    q->head = q->tail = q->size = 0;                                        \
    return q;                                                               \
}                                                                           \
                                                                            \
bool Queue_##T##_Deque(Queue_##T *q, T *out) {                              \
    if (q->size == 0) return false;                                         \
    *out = q->data[q->head];                                                \
    q->head = (q->head + 1) & q->mask;                                      \
    --q->size;                                                              \
    return true;                                                            \
}                                                                           \
                                                                            \
void Queue_##T##_Free(Queue_##T *pq) {                                     \
    free(pq->data);                                                      \
    free(pq);                                                              \
}                                                                                                                                                \


#endif