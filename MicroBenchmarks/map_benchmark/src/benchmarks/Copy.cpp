#include "Map.h"
#include "benchmark/benchmark.h"
#include "sfc64.h"
#include <cstdint>

void Copy(benchmark::State &state) {
    size_t result = 0;

    sfc64 rng(987);

    using M = Map<uint64_t, uint64_t>;
#ifdef USE_POOL_ALLOCATOR
    Resource<uint64_t, uint64_t> resource;
    M mapSource{0, M::hasher{}, M::key_equal{}, &resource};
#else
    M mapSource;
#endif
    uint64_t rememberKey = 0;
    for (size_t i = 0; i < 10000; ++i) {
        auto key = rng();
        if (i == 5000) {
            rememberKey = key;
        }
        mapSource[key] = i;
    }

    M mapForCopy = mapSource;
    
    for(auto _ : state) {
        M m = mapForCopy;
        result += m.size() + m[rememberKey];
        m[rng()] = rng();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(Copy);
