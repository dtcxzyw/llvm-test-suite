#include "Map.h"
#include "benchmark/benchmark.h"
#include "sfc64.h"

void IterateIntegers(benchmark::State &state) {
    sfc64 rng(123);

    size_t const num_iters = 500;

    uint64_t result = 0;

    for(auto _ : state) {
        using M = Map<uint64_t, uint64_t>;
#ifdef USE_POOL_ALLOCATOR
        Resource<uint64_t, uint64_t> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif

        auto const state = rng.state();
        for (size_t n = 0; n < num_iters; ++n) {
            map[rng()] = n;
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        }

        rng.state(state);
        for (size_t n = 0; n < num_iters; ++n) {
            map.erase(rng());
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        }

        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(IterateIntegers);
