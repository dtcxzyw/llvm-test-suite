#include "Map.h"
#include "benchmark/benchmark.h"

void CtorDtorEmptyMap(benchmark::State &state) {
    size_t result = 0;
    for(auto _ : state) {
        using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
        Resource<int, int> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif
        result += map.size();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(CtorDtorEmptyMap);

void CtorDtorSingleEntryMap(benchmark::State &state) {
    size_t result = 0;
    int n = 0;
    for(auto _ : state) {
        using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
        Resource<int, int> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif
        map[n++];
        result += map.size();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(CtorDtorSingleEntryMap);
