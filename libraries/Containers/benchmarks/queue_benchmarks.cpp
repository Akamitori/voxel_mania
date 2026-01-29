#include <cstdio>
#include <deque>
#include <chrono>
#include <random>
#include "queue_container.h"

// Instantiate the queue implementation for int
QUEUE_DECLARATION_STATIC(int)
QUEUE_IMPLEMENTATION(int)

using namespace std;
using namespace std::chrono;

// ------------------------------------------------------------ 
// Benchmark helper
// ------------------------------------------------------------ 
template<typename Func>
double benchmark(Func &&f, const char *name) {
    auto start = high_resolution_clock::now();
    f();
    auto end = high_resolution_clock::now();
    double ms = duration_cast<microseconds>(end - start).count() / 1000.0;
    printf("%-45s: %10.3f ms\n", name, ms);
    return ms;
}

// ------------------------------------------------------------ 
// Helpers
// ------------------------------------------------------------ 
static inline bool deque_enqueue(deque<int>& q, int v) {
    q.push_back(v);
    return true;
}

static inline bool deque_dequeue(deque<int>& q, int& out) {
    if (q.empty()) return false;
    out = q.front();
    q.pop_front();
    return true;
}

// ------------------------------------------------------------ 
// Test 1: Sequential enqueue then dequeue
// ------------------------------------------------------------ 
void test_sequential(int N) {
    printf("\n=== Sequential (N=%d) ===\n", N);
    
    benchmark([&]() {
        deque<int> q;
        for (int i = 0; i < N; ++i) deque_enqueue(q, i);
        int v;
        while (deque_dequeue(q, v)) {}
    }, "std::deque");
    
    benchmark([&]() {
        Queue_int* q = Queue_int_Create(N);
        if (!q) { printf("ERROR: Create failed\n"); return; }
        for (int i = 0; i < N; ++i) Queue_int_Enqueue(q, i);
        int v;
        while (Queue_int_Deque(q, &v)) {}
        Queue_int_Free(q);
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// Test 2: Alternating workload
// ------------------------------------------------------------ 
void test_alternating(int N) {
    printf("\n=== Alternating (N=%d) ===\n", N);
    
    benchmark([&]() {
        deque<int> q;
        int v;
        for (int i = 0; i < N; ++i) {
            deque_enqueue(q, i);
            if (i % 3 == 0) deque_dequeue(q, v);
        }
        while (deque_dequeue(q, v)) {}
    }, "std::deque");
    
    benchmark([&]() {
        Queue_int* q = Queue_int_Create(N);
        if (!q) { printf("ERROR: Create failed\n"); return; }
        int v;
        for (int i = 0; i < N; ++i) {
            Queue_int_Enqueue(q, i);
            if (i % 3 == 0) Queue_int_Deque(q, &v);
        }
        while (Queue_int_Deque(q, &v)) {}
        Queue_int_Free(q);
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// Test 3: Wraparound stress (fixed capacity, no overflow)
// ------------------------------------------------------------ 
void test_wraparound(int N) {
    printf("\n=== Wraparound (N=%d) ===\n", N);
    const int capacity = N / 2 + 1;
    
    benchmark([&]() {
        deque<int> q;
        int v;
        for (int i = 0; i < capacity; ++i) deque_enqueue(q, i);
        for (int i = 0; i < N; ++i) {
            deque_dequeue(q, v);
            deque_enqueue(q, i);
        }
    }, "std::deque");
    
    benchmark([&]() {
        Queue_int* q = Queue_int_Create(capacity);
        if (!q) { printf("ERROR: Create failed\n"); return; }
        int v;
        for (int i = 0; i < capacity; ++i) Queue_int_Enqueue(q, i);
        for (int i = 0; i < N; ++i) {
            Queue_int_Deque(q, &v);
            Queue_int_Enqueue(q, i);
        }
        Queue_int_Free(q);
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// Test 4: Random operations (capacity-aware)
// ------------------------------------------------------------ 
void test_random(int N) {
    printf("\n=== Random Ops (N=%d) ===\n", N);
    mt19937 gen(42);
    uniform_int_distribution<> dist(0, 1);
    
    benchmark([&]() {
        deque<int> q;
        gen.seed(42);
        int v;
        for (int i = 0; i < N; ++i) {
            if (dist(gen) == 0 || q.empty()) {
                deque_enqueue(q, i);
            } else {
                deque_dequeue(q, v);
            }
        }
    }, "std::deque");
    
    benchmark([&]() {
        Queue_int* q = Queue_int_Create(N);
        if (!q) { printf("ERROR: Create failed\n"); return; }
        gen.seed(42);
        int v;
        for (int i = 0; i < N; ++i) {
            if (dist(gen) == 0 || q->size == 0) {
                Queue_int_Enqueue(q, i);
            } else {
                Queue_int_Deque(q, &v);
            }
        }
        Queue_int_Free(q);
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// Test 5: Many small queues (allocation stress)
// ------------------------------------------------------------ 
void test_many_small(int num_queues, int size_each) {
    printf("\n=== Many Small Queues (n=%d, size=%d) ===\n", num_queues, size_each);
    
    benchmark([&]() {
        for (int q = 0; q < num_queues; ++q) {
            deque<int> dq;
            int v;
            for (int i = 0; i < size_each; ++i) deque_enqueue(dq, i);
            while (deque_dequeue(dq, v)) {}
        }
    }, "std::deque");
    
    benchmark([&]() {
        for (int q = 0; q < num_queues; ++q) {
            Queue_int* qu = Queue_int_Create(size_each);
            if (!qu) continue;
            int v;
            for (int i = 0; i < size_each; ++i) Queue_int_Enqueue(qu, i);
            while (Queue_int_Deque(qu, &v)) {}
            Queue_int_Free(qu);
        }
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// Test 6: Steady-state (pre-filled queue, sustained ops)
// ------------------------------------------------------------ 
void test_steady_state(int N) {
    printf("\n=== Steady-State (N=%d) ===\n", N);
    
    benchmark([&]() {
        deque<int> q;
        // Pre-fill
        for (int i = 0; i < N; ++i) deque_enqueue(q, i);
        // Sustained load: dequeue and enqueue
        int v;
        for (int i = 0; i < N; ++i) {
            deque_dequeue(q, v);
            deque_enqueue(q, i + N);
        }
    }, "std::deque");
    
    benchmark([&]() {
        Queue_int* q = Queue_int_Create(N);
        if (!q) { printf("ERROR: Create failed\n"); return; }
        // Pre-fill
        for (int i = 0; i < N; ++i) Queue_int_Enqueue(q, i);
        // Sustained load: dequeue and enqueue
        int v;
        for (int i = 0; i < N; ++i) {
            Queue_int_Deque(q, &v);
            Queue_int_Enqueue(q, i + N);
        }
        Queue_int_Free(q);
    }, "Queue_int");
}

// ------------------------------------------------------------ 
// main
// ------------------------------------------------------------ 
int main() {
    printf("Queue Benchmark: Fixed-Capacity Queue vs std::deque\n");
    printf("====================================================\n\n");
    
    test_sequential(100000);
    test_sequential(1000000);
    
    test_alternating(100000);
    test_alternating(1000000);
    
    test_wraparound(100000);
    test_wraparound(1000000);
    
    test_random(100000);
    test_random(1000000);
    
    test_many_small(10000, 10);
    test_many_small(1000, 100);
    
    test_steady_state(1000000);
    
    printf("\n====================================================\n");
    printf("All tests completed successfully!\n");
    
    return 0;
}