#include "Map.h"
#include "benchmark/benchmark.h"
#include "hex.h"
#include "sfc64.h"
#include "shuffle.h"

#include <algorithm>
#include <bitset>
#include <numeric>
#include <sstream>

template <typename T>
struct as_bits_t {
    T value;
};

template <typename T>
as_bits_t<T> as_bits(T value) {
    return as_bits_t<T>{value};
}

template <typename T>
std::ostream& operator<<(std::ostream& os, as_bits_t<T> const& t) {
    os << std::bitset<sizeof(t.value) * 8>(t.value);
    return os;
}

void RandomInsertErase(benchmark::State &state) {
    // random bits to set for the mask
    std::vector<int> bits(64);
    std::iota(bits.begin(), bits.end(), 0);
    sfc64 rng(999);
    slightlyBiasedShuffle(bits.begin(), bits.end(), rng);

    size_t const expectedFinalSizes[] = {7, 123, 2031, 32800, 524439, 8366634};
    size_t const max_n = 5000;

    for(auto _ : state) {
        using M = Map<uint64_t, uint64_t>;
#ifdef USE_POOL_ALLOCATOR
        Resource<uint64_t, uint64_t> resource;
        M map{0, M::hasher{}, M::key_equal{}, &resource};
#else
        M map;
#endif

        uint64_t bitMask = 0;

        auto bitsIt = bits.begin();
        for (int i = 0; i < 6; ++i) {
            // each iteration, set 4 new random bits.
            for (int b = 0; b < 4; ++b) {
                bitMask |= UINT64_C(1) << *bitsIt++;
            }

            // benchmark randomly inserting & erasing
            for (size_t i = 0; i < max_n; ++i) {
                map.emplace(rng() & bitMask, i);
                map.erase(rng() & bitMask);
            }
        }
    }
}


BENCHMARK(RandomInsertErase);
