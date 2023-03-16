#include "Map.h"
#include "benchmark/benchmark.h"
#include "sfc64.h"

#include <sstream>

size_t run(size_t max_n, size_t string_length, uint32_t bitMask) {
    sfc64 rng(123);

    // time measured part
    size_t verifier = 0;

    std::string str(string_length, 'x');
    // point to the back of the string (32bit aligned), so comparison takes a while
    auto const idx32 = (string_length / 4) - 1;
    auto const strData32 = reinterpret_cast<uint32_t*>(&str[0]) + idx32;

    using M = Map<std::string, std::string>;
#ifdef USE_POOL_ALLOCATOR
    Resource<std::string, std::string> resource;
    M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
    M map;
#endif
    for (size_t i = 0; i < max_n; ++i) {
        *strData32 = rng() & bitMask;

        // create an entry.
        map[str];

        *strData32 = rng() & bitMask;
        auto it = map.find(str);
        if (it != map.end()) {
            ++verifier;
            map.erase(it);
        }
    }
    return verifier;
}

void RandomInsertEraseStrings(benchmark::State &state) {
    for(auto _ : state) {
        benchmark::DoNotOptimize(run(20000, 7, 0xfffff));
        benchmark::DoNotOptimize(run(20000, 8, 0xfffff));
        benchmark::DoNotOptimize(run(20000, 13, 0xfffff));
        benchmark::DoNotOptimize(run(12000, 100, 0x7ffff));
        benchmark::DoNotOptimize(run(6000, 1000, 0x1ffff));
    }
}

BENCHMARK(RandomInsertEraseStrings);
