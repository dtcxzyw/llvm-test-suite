#include "Map.h"
#include "benchmark/benchmark.h"
#include "hex.h"
#include "sfc64.h"
#include "shuffle.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

uint64_t randomFindInternal(size_t numRandom, uint64_t bitMask, size_t numInserts, size_t numFindsPerInsert) {
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

    {
        using M = Map<size_t, size_t>;
#ifdef USE_POOL_ALLOCATOR
        Resource<size_t, size_t> resource;
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
                    map[rng() & bitMask] = static_cast<size_t>(1);
                } else {
                    map[val & bitMask] = static_cast<size_t>(1);
                }
                ++i;
            }

            // the actual benchmark code which sohould be as fast as possible
            for (size_t j = 0; j < numFindsPerIter; ++j) {
                if (++findCount > i) {
                    findCount = 0;
                    findRng.state(anotherUnrelatedRngInitialState);
                }
                auto it = map.find(findRng() & bitMask);
                if (it != map.end()) {
                    num_found += it->second;
                }
            }
        } while (i < numInserts);
    }
    return num_found;
}

void RandomFind(benchmark::State &state) {
    static constexpr auto lower32bit = UINT64_C(0x00000000FFFFFFFF);
    static constexpr auto upper32bit = UINT64_C(0xFFFFFFFF00000000);


    for(auto _ : state) {
        size_t numInserts = state.range(0);
        size_t numFindsPerInsert = state.range(1);
        
        benchmark::DoNotOptimize(randomFindInternal(4, lower32bit, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternal(4, upper32bit, numInserts, numFindsPerInsert));

        benchmark::DoNotOptimize(randomFindInternal(3, lower32bit, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternal(3, upper32bit, numInserts, numFindsPerInsert));

        benchmark::DoNotOptimize(randomFindInternal(2, lower32bit, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternal(2, upper32bit, numInserts, numFindsPerInsert));

        benchmark::DoNotOptimize(randomFindInternal(1, lower32bit, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternal(1, upper32bit, numInserts, numFindsPerInsert));

        benchmark::DoNotOptimize(randomFindInternal(0, lower32bit, numInserts, numFindsPerInsert));
        benchmark::DoNotOptimize(randomFindInternal(0, upper32bit, numInserts, numFindsPerInsert));
    }
}

BENCHMARK(RandomFind)->Args({20,5000})->Args({200,500})->Args({5000,10});
