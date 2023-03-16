#include "Map.h"
#include "benchmark/benchmark.h"
#include "hex.h"
#include "sfc64.h"
#include "shuffle.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

uint64_t randomFindInternalString(size_t numRandom, size_t const length, size_t numInserts, size_t numFindsPerInsert) {
    size_t constexpr NumTotal = 4;
    size_t const numSequential = NumTotal - numRandom;

    size_t const numFindsPerIter = numFindsPerInsert * NumTotal;

    sfc64 rng(123);

    size_t num_found = 0;

    std::array<bool, NumTotal> insertRandom = {false};
    for (size_t i = 0; i < numRandom; ++i) {
        insertRandom[i] = true;
    }

    sfc64 anotherUnrelatedRng(987654321);
    auto const anotherUnrelatedRngInitialState = anotherUnrelatedRng.state();
    sfc64 findRng(anotherUnrelatedRngInitialState);

    std::string str(length, 'y');
    // point to the back of the string (32bit aligned), so comparison takes a while
    auto const idx32 = (length / 4) - 1;
    auto const strData32 = reinterpret_cast<uint32_t*>(&str[0]) + idx32;

    {
        using M = Map<std::string, size_t>;
#ifdef USE_POOL_ALLOCATOR
        Resource<std::string, size_t> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif
        size_t i = 0;
        size_t findCount = 0;

        do {

            // insert NumTotal entries: some random, some sequential.
            slightlyBiasedShuffle(insertRandom.begin(), insertRandom.end(), rng);
            for (bool isRandomToInsert : insertRandom) {
                auto val = anotherUnrelatedRng();
                if (isRandomToInsert) {
                    *strData32 = rng();
                } else {
                    *strData32 = val;
                }
                map[str] = static_cast<size_t>(1);
                ++i;
            }

            // the actual benchmark code which sohould be as fast as possible
            for (size_t j = 0; j < numFindsPerIter; ++j) {
                if (++findCount > i) {
                    findCount = 0;
                    findRng.state(anotherUnrelatedRngInitialState);
                }
                *strData32 = findRng();
                auto it = map.find(str);
                if (it != map.end()) {
                    num_found += it->second;
                }
            }
        } while (i < numInserts);
    }
    return num_found;
}

void RandomFindString(benchmark::State &state) {
    for(auto _ : state) {
        size_t numInserts = state.range(0);
        size_t numFindsPerInsert = state.range(1);
        const auto length = state.range(2);

        benchmark::DoNotOptimize(randomFindInternalString(4, length, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternalString(3, length, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternalString(2, length, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternalString(1, length, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternalString(0, length, numInserts, numFindsPerInsert));
    }
}

BENCHMARK(RandomFindString)->Args({100,100,10})->Args({1000,20,13});
