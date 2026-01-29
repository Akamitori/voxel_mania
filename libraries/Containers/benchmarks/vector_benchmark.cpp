#include <cstdio>
#include <vector>
#include <chrono>

// If VECTOR_USE_LIBRARY is defined, use the library version (linked)
// Otherwise, use the static inline version (header-only)
#ifdef VECTOR_USE_LIBRARY
    #include "vector_int.h"
#else
    #include "vector_container.h"
    VECTOR_IMPLEMENTATION_STATIC(int)
#endif

using namespace std;
using namespace std::chrono;

constexpr int BENCHMARK_RUNS = 100;

template<typename Func>
double benchmark_single(Func &&f) {
    auto start = high_resolution_clock::now();
    f();
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

template<typename Func>
double benchmark(Func &&f, const char *name) {
    double total = 0;
    for (int r = 0; r < BENCHMARK_RUNS; ++r) {
        total += benchmark_single(f);
    }
    double avg = total / BENCHMARK_RUNS;
    printf("%-45s: %10.3f ms\n", name, avg);
    return avg;
}

void test_sequential_append(int N) {
    printf("\n=== Sequential Append (N=%d) ===\n", N);
    
    benchmark([&]() {
        vector<int> v;
        for (int i = 0; i < N; ++i) v.push_back(i);
    }, "std::vector");
    
    benchmark([&]() {
        Vector_int* v = Vector_int_Create(N);
        if (!v) return;
        for (int i = 0; i < N; ++i) Vector_int_Add(v, i);
        Vector_int_Free(v);
    }, "Vector_int");
}

void test_reserved_capacity(int N) {
    printf("\n=== Pre-Reserved Capacity (N=%d) ===\n", N);
    
    benchmark([&]() {
        vector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; ++i) v.push_back(i);
    }, "std::vector");
    
    benchmark([&]() {
        Vector_int* v = Vector_int_Create(N);
        if (!v) return;
        for (int i = 0; i < N; ++i) Vector_int_Add(v, i);
        Vector_int_Free(v);
    }, "Vector_int");
}

void test_repeated_clear_refill(int N, int iterations) {
    printf("\n=== Repeated Clear & Refill (N=%d, iter=%d) ===\n", N, iterations);
    
    vector<int> sv;
    sv.reserve(N);
    
    Vector_int* vp = Vector_int_Create(N);
    
    benchmark([&]() {
        for (int iter = 0; iter < iterations; ++iter) {
            sv.clear();
            for (int i = 0; i < N; ++i) sv.push_back(i);
        }
    }, "std::vector");
    
    benchmark([&]() {
        for (int iter = 0; iter < iterations; ++iter) {
            Vector_int_Clear(vp);
            for (int i = 0; i < N; ++i) Vector_int_Add(vp, i);
        }
    }, "Vector_int");
    
    Vector_int_Free(vp);
}

void test_many_small_vectors(int num_vectors, int size_each) {
    printf("\n=== Many Small Vectors (n=%d, size=%d) ===\n", num_vectors, size_each);
    
    benchmark([&]() {
        for (int v = 0; v < num_vectors; ++v) {
            vector<int> vec;
            for (int i = 0; i < size_each; ++i) vec.push_back(i);
        }
    }, "std::vector");
    
    benchmark([&]() {
        for (int v = 0; v < num_vectors; ++v) {
            Vector_int* vec = Vector_int_Create(size_each);
            if (!vec) continue;
            for (int i = 0; i < size_each; ++i) Vector_int_Add(vec, i);
            Vector_int_Free(vec);
        }
    }, "Vector_int");
}

void test_mixed_workload(int N) {
    printf("\n=== Mixed Workload (N=%d) ===\n", N);
    
    benchmark([&]() {
        vector<int> v;
        for (int i = 0; i < N; ++i) {
            v.push_back(i);
            if (i % 5 == 0) {
                volatile int x = v[i / 2];
                (void)x;
            }
            if (i % 3 == 0 && !v.empty()) {
                v[i % v.size()] = i * 2;
            }
        }
    }, "std::vector");
    
    benchmark([&]() {
        Vector_int* v = Vector_int_Create(N);
        if (!v) return;
        for (int i = 0; i < N; ++i) {
            Vector_int_Add(v, i);
            int len = Vector_int_Length(v);
            if (i % 5 == 0) {
                volatile int x = v->data[i / 2];
                (void)x;
            }
            if (i % 3 == 0 && len > 0) {
                v->data[i % len] = i * 2;
            }
        }
        Vector_int_Free(v);
    }, "Vector_int");
}

void test_exponential_growth(int N) {
    printf("\n=== Exponential Growth (N=%d elements) ===\n", N);
    
    benchmark([&]() {
        vector<int> v;
        int count = 0;
        for (int i = 0; count < N; ++i) {
            for (int j = 0; j < (1 << i) && count < N; ++j) {
                v.push_back(count++);
            }
        }
    }, "std::vector");
    
    benchmark([&]() {
        Vector_int* v = Vector_int_Create(8);
        if (!v) return;
        int count = 0;
        for (int i = 0; count < N; ++i) {
            for (int j = 0; j < (1 << i) && count < N; ++j) {
                Vector_int_Add(v, count++);
            }
        }
        Vector_int_Free(v);
    }, "Vector_int");
}

int main() {
#ifdef VECTOR_USE_LIBRARY
    printf("VectorPtr Benchmark (LIBRARY version)\n");
#else
    printf("VectorPtr Benchmark (STATIC INLINE version)\n");
#endif
    printf("(Each test averaged over %d runs)\n", BENCHMARK_RUNS);
    printf("=====================================================\n");
    
    test_sequential_append(100000);
    test_sequential_append(1000000);
    
    test_reserved_capacity(100000);
    test_reserved_capacity(1000000);
    
    test_repeated_clear_refill(100000, 100);
    test_repeated_clear_refill(1000000, 10);
    
    test_many_small_vectors(10000, 10);
    test_many_small_vectors(1000, 100);
    
    test_mixed_workload(100000);
    test_mixed_workload(1000000);
    
    test_exponential_growth(100000);
    
    printf("\n=====================================================\n");
    printf("All tests completed successfully!\n");
    
    return 0;
}