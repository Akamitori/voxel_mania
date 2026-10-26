#include <cstdio>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "hashmap_container.h"

// ------------------------------------------------------------
// Instantiate the hashmap for int -> int
// ------------------------------------------------------------
static uint64_t hm_fnv1a(const uint8_t *data, size_t len) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) { h ^= data[i]; h *= 1099511628211ULL; }
    return h;
}

HASHMAP_IMPLEMENTATION_STATIC(int, int)

static int      int_int_Hashmap_key_eq(int a, int b) { return a == b; }
static uint64_t int_int_Hashmap_key_hash(int k)      { return hm_fnv1a((const uint8_t *)&k, sizeof k); }

// Same hash for std::unordered_map, so we compare table designs, not hash functions
struct FnvHash { size_t operator()(int k) const { return (size_t)int_int_Hashmap_key_hash(k); } };
typedef std::unordered_map<int, int, FnvHash> StdMap;

static const float MAX_LOAD = 0.85f;

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
static inline void hm_insert(int_int_hashmap *h, int k, int v) {
    int_int_hashmap_pair p = { k, v };
    int_int_Hashmap_Insert(h, p);
}

// Both implementations compute the same checksum; a mismatch means a bug.
static void verify(long long expected, long long got, const char *what) {
    if (expected != got) printf("  !! MISMATCH in %s: std %lld vs hashmap %lld\n", what, expected, got);
}

// Unique, well-scattered keys. First n are "present", next n are "absent".
static vector<int> make_keys(int n) {
    vector<int> keys(2 * (size_t)n);
    for (size_t i = 0; i < keys.size(); ++i) keys[i] = (int)(i * 2654435761u);
    mt19937 gen(1234);
    shuffle(keys.begin(), keys.end(), gen);
    return keys;
}

// ------------------------------------------------------------
// Test 1: Insert N keys into an empty map (includes growth)
// ------------------------------------------------------------
void test_insert(int N) {
    printf("\n=== Insert with growth (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);
    size_t size_std = 0, size_hm = 0;

    benchmark([&]() {
        StdMap m;
        for (int i = 0; i < N; ++i) m[keys[i]] = i;
        size_std = m.size();
    }, "std::unordered_map");

    benchmark([&]() {
        int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
        if (!h) { printf("ERROR: Create failed\n"); return; }
        for (int i = 0; i < N; ++i) hm_insert(h, keys[i], i);
        size_hm = h->num_elements;
        int_int_Hashmap_Free(h);
    }, "Hashmap int_int");

    verify((long long)size_std, (long long)size_hm, "insert size");
}

// ------------------------------------------------------------
// Test 2: Insert N keys into a pre-reserved map (no growth)
// ------------------------------------------------------------
void test_insert_reserved(int N) {
    printf("\n=== Insert reserved (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);

    benchmark([&]() {
        StdMap m;
        m.reserve(N);
        for (int i = 0; i < N; ++i) m[keys[i]] = i;
    }, "std::unordered_map");

    benchmark([&]() {
        int capacity = (int)(N / MAX_LOAD) + 1;
        int_int_hashmap *h = int_int_Hashmap_Create(capacity, MAX_LOAD);
        if (!h) { printf("ERROR: Create failed\n"); return; }
        for (int i = 0; i < N; ++i) hm_insert(h, keys[i], i);
        int_int_Hashmap_Free(h);
    }, "Hashmap int_int");
}

// ------------------------------------------------------------
// Test 3: Lookups that hit (random order)
// ------------------------------------------------------------
void test_lookup_hits(int N) {
    printf("\n=== Lookup hits (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);
    vector<int> order(keys.begin(), keys.begin() + N);
    mt19937 gen(42);
    shuffle(order.begin(), order.end(), gen);

    StdMap m;
    int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
    for (int i = 0; i < N; ++i) { m[keys[i]] = i; hm_insert(h, keys[i], i); }

    long long sum_std = 0, sum_hm = 0;

    benchmark([&]() {
        for (int k : order) { auto it = m.find(k); if (it != m.end()) sum_std += it->second; }
    }, "std::unordered_map");

    benchmark([&]() {
        for (int k : order) { int *v = int_int_Hashmap_Find(h, k); if (v) sum_hm += *v; }
    }, "Hashmap int_int");

    verify(sum_std, sum_hm, "lookup hits");
    int_int_Hashmap_Free(h);
}

// ------------------------------------------------------------
// Test 4: Lookups that miss
// ------------------------------------------------------------
void test_lookup_misses(int N) {
    printf("\n=== Lookup misses (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);

    StdMap m;
    int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
    for (int i = 0; i < N; ++i) { m[keys[i]] = i; hm_insert(h, keys[i], i); }

    long long found_std = 0, found_hm = 0;

    benchmark([&]() {
        for (int i = N; i < 2 * N; ++i) found_std += m.find(keys[i]) != m.end();
    }, "std::unordered_map");

    benchmark([&]() {
        for (int i = N; i < 2 * N; ++i) found_hm += int_int_Hashmap_Find(h, keys[i]) != NULL;
    }, "Hashmap int_int");

    verify(found_std, found_hm, "lookup misses");
    if (found_hm != 0) printf("  !! hashmap found %lld keys that were never inserted\n", found_hm);
    int_int_Hashmap_Free(h);
}

// ------------------------------------------------------------
// Test 5: Remove every key (random order)
// ------------------------------------------------------------
void test_remove_all(int N) {
    printf("\n=== Remove all (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);
    vector<int> order(keys.begin(), keys.begin() + N);
    mt19937 gen(7);
    shuffle(order.begin(), order.end(), gen);

    StdMap m;
    int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
    for (int i = 0; i < N; ++i) { m[keys[i]] = i; hm_insert(h, keys[i], i); }

    benchmark([&]() {
        for (int k : order) m.erase(k);
    }, "std::unordered_map");

    benchmark([&]() {
        for (int k : order) int_int_Hashmap_Remove(h, k);
    }, "Hashmap int_int");

    verify((long long)m.size(), (long long)h->num_elements, "size after remove all");
    int_int_Hashmap_Free(h);
}

// ------------------------------------------------------------
// Test 6: Random mixed ops (50% insert, 25% remove, 25% find)
// ------------------------------------------------------------
void test_random_mixed(int N) {
    printf("\n=== Random Mixed Ops (N=%d) ===\n", N);
    mt19937 gen(42);
    uniform_int_distribution<> op_dist(0, 3);
    uniform_int_distribution<> key_dist(0, N / 4);   // small key range: lots of hits and updates

    long long sum_std = 0, sum_hm = 0;
    size_t size_std = 0, size_hm = 0;

    benchmark([&]() {
        StdMap m;
        gen.seed(42);
        for (int i = 0; i < N; ++i) {
            int op = op_dist(gen), k = key_dist(gen);
            if (op <= 1)      m[k] = i;
            else if (op == 2) m.erase(k);
            else { auto it = m.find(k); if (it != m.end()) sum_std += it->second; }
        }
        size_std = m.size();
    }, "std::unordered_map");

    benchmark([&]() {
        int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
        if (!h) { printf("ERROR: Create failed\n"); return; }
        gen.seed(42);
        for (int i = 0; i < N; ++i) {
            int op = op_dist(gen), k = key_dist(gen);
            if (op <= 1)      hm_insert(h, k, i);
            else if (op == 2) int_int_Hashmap_Remove(h, k);
            else { int *v = int_int_Hashmap_Find(h, k); if (v) sum_hm += *v; }
        }
        size_hm = h->num_elements;
        int_int_Hashmap_Free(h);
    }, "Hashmap int_int");

    verify(sum_std, sum_hm, "mixed lookup sums");
    verify((long long)size_std, (long long)size_hm, "mixed final size");
}

// ------------------------------------------------------------
// Test 7: Many small maps (allocation stress)
// ------------------------------------------------------------
void test_many_small(int num_maps, int size_each) {
    printf("\n=== Many Small Maps (n=%d, size=%d) ===\n", num_maps, size_each);
    long long sum_std = 0, sum_hm = 0;

    benchmark([&]() {
        for (int m = 0; m < num_maps; ++m) {
            StdMap map;
            for (int i = 0; i < size_each; ++i) map[i * 7919] = i;
            for (int i = 0; i < size_each; ++i) sum_std += map.find(i * 7919)->second;
        }
    }, "std::unordered_map");

    benchmark([&]() {
        for (int m = 0; m < num_maps; ++m) {
            int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
            if (!h) continue;
            for (int i = 0; i < size_each; ++i) hm_insert(h, i * 7919, i);
            for (int i = 0; i < size_each; ++i) sum_hm += *int_int_Hashmap_Find(h, i * 7919);
            int_int_Hashmap_Free(h);
        }
    }, "Hashmap int_int");

    verify(sum_std, sum_hm, "many small sums");
}

// ------------------------------------------------------------
// Test 8: Steady-state churn (entities despawning / spawning)
// Pre-fill N keys, then repeatedly remove the oldest and add a new one.
// ------------------------------------------------------------
void test_steady_state(int N) {
    printf("\n=== Steady-State Churn (N=%d) ===\n", N);
    vector<int> keys = make_keys(N);   // 2N unique keys: N live + N to spawn
    size_t size_std = 0, size_hm = 0;

    benchmark([&]() {
        StdMap m;
        for (int i = 0; i < N; ++i) m[keys[i]] = i;
        for (int i = 0; i < N; ++i) {
            m.erase(keys[i]);
            m[keys[N + i]] = i;
        }
        size_std = m.size();
    }, "std::unordered_map");

    benchmark([&]() {
        int_int_hashmap *h = int_int_Hashmap_Create(8, MAX_LOAD);
        if (!h) { printf("ERROR: Create failed\n"); return; }
        for (int i = 0; i < N; ++i) hm_insert(h, keys[i], i);
        for (int i = 0; i < N; ++i) {
            int_int_Hashmap_Remove(h, keys[i]);
            hm_insert(h, keys[N + i], i);
        }
        size_hm = h->num_elements;
        int_int_Hashmap_Free(h);
    }, "Hashmap int_int");

    verify((long long)size_std, (long long)size_hm, "steady-state size");
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main() {
    printf("Hashmap Benchmark: Robin Hood Hashmap vs std::unordered_map\n");
    printf("===========================================================\n\n");

    test_insert(100000);
    test_insert(1000000);

    test_insert_reserved(100000);
    test_insert_reserved(1000000);

    test_lookup_hits(100000);
    test_lookup_hits(1000000);

    test_lookup_misses(100000);
    test_lookup_misses(1000000);

    test_remove_all(100000);
    test_remove_all(1000000);

    test_random_mixed(100000);
    test_random_mixed(1000000);

    test_many_small(10000, 10);
    test_many_small(1000, 100);

    test_steady_state(1000000);

    printf("\n===========================================================\n");
    printf("All tests completed successfully!\n");

    return 0;
}