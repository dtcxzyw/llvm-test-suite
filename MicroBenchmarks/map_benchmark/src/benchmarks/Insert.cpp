#include "Map.h"
#include "benchmark/benchmark.h"
#include "sfc64.h"

void InsertHugeInt(benchmark::State &state) {
    sfc64 rng(213);

    size_t result = 0;

    for(auto _ : state) {
        using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
        Resource<int, int> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif
        for (size_t n = 0; n < 100000; ++n) {
            map[static_cast<int>(rng())];
        }

        map.clear();

        // remember the rng's state so we can remove like we've added
        auto const state = rng.state();
        for (size_t n = 0; n < 100000; ++n) {
            map[static_cast<int>(rng())];
        }

        rng.state(state);
        for (size_t n = 0; n < 100000; ++n) {
            map.erase(static_cast<int>(rng()));
        }

        result += map.size();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(InsertHugeInt);
