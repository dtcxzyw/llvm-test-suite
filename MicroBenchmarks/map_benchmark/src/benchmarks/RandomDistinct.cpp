#include "Map.h"
#include "benchmark/benchmark.h"
#include "sfc64.h"

void RandomDistinct2(benchmark::State &state) {
    static size_t const n = 50000;

    sfc64 rng(123);

    for(auto _ : state) {
        int checksum = 0;
        {
                size_t const max_rng = n / 20;

                using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
                Resource<int, int> resource;
                M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
                M map;
#endif
                for (size_t i = 0; i < n; ++i) {
                checksum += ++map[static_cast<int>(rng(max_rng))];
                }
        }

        {
                size_t const max_rng = n / 4;

                using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
                Resource<int, int> resource;
                M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
                M map;
#endif
                for (size_t i = 0; i < n; ++i) {
                checksum += ++map[static_cast<int>(rng(max_rng))];
                }
        }

        {
                size_t const max_rng = n / 2;

                using M = Map<int, int>;
#ifdef USE_POOL_ALLOCATOR
                Resource<int, int> resource;
                M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
                M map;
#endif
                for (size_t i = 0; i < n; ++i) {
                checksum += ++map[static_cast<int>(rng(max_rng))];
                }
        }
        {
                using M = Map<int, int>;
        #ifdef USE_POOL_ALLOCATOR
                Resource<int, int> resource;
                M map{0, M::hasher{}, M::key_equal{}, &resource};
        #else
                M map;
        #endif
                for (size_t i = 0; i < n; ++i) {
                checksum += ++map[static_cast<int>(rng())];
                }
        }
        
        benchmark::DoNotOptimize(checksum);
    }
}

BENCHMARK(RandomDistinct2);
