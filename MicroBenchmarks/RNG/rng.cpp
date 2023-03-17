#include "benchmark/benchmark.h"
#include <cstdint>
#include <random>
#include "pcg_random.hpp"

extern "C" {
void initISAAC();
unsigned int genISAAC();
}

class ISAAC final {
public:
	using result_type = unsigned int;

	ISAAC() {
		initISAAC();
	}

	[[nodiscard]] static constexpr result_type(min)() {
		return 0;
	}

	[[nodiscard]] static constexpr result_type(max)() {
		return static_cast<result_type>(-1);
	}

	[[nodiscard]] result_type operator()() {
		return genISAAC();
	}
};


// See also https://prng.di.unimi.it/
namespace {
    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    static uint64_t x = 0; /* The state can be seeded with any value. */

    uint64_t next() {
        uint64_t z = (x += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }

    class Xoroshiro128pp {
        uint64_t s[2];

    public:
        using result_type = uint64_t;
        Xoroshiro128pp() {
            s[0] = next();
            s[1] = next();
        }

        [[nodiscard]] static constexpr result_type(min)() {
            return 0;
        }

        [[nodiscard]] static constexpr result_type(max)() {
            return static_cast<result_type>(-1);
        }

        [[nodiscard]] result_type operator()() {
            const uint64_t s0 = s[0];
            uint64_t s1 = s[1];
            const uint64_t result = rotl(s0 + s1, 17) + s0;

            s1 ^= s0;
            s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21);  // a, b
            s[1] = rotl(s1, 28);                    // c

            return result;
        }
    };

    class Xoshiro256pp {
        uint64_t s[4];

    public:
        using result_type = uint64_t;
        Xoshiro256pp() {
            s[0] = next();
            s[1] = next();
            s[2] = next();
            s[3] = next();
        }

        [[nodiscard]] static constexpr result_type(min)() {
            return 0;
        }

        [[nodiscard]] static constexpr result_type(max)() {
            return static_cast<result_type>(-1);
        }

        [[nodiscard]] result_type operator()() {
            const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;

            s[3] = rotl(s[3], 45);

            return result;
        }
    };

}  // namespace

// See also https://gist.github.com/orlp/32f5d1b631ab092608b1

/*
    Copyright (c) 2015 Orson Peters <orsonpeters@gmail.com>
    
    This software is provided 'as-is', without any express or implied warranty. In no event will the
    authors be held liable for any damages arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose, including commercial
    applications, and to alter it and redistribute it freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the
       original software. If you use this software in a product, an acknowledgment in the product
       documentation would be appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as
       being the original software.
    
    3. This notice may not be removed or altered from any source distribution.
*/
namespace {
	template<size_t R>
	class ChaCha {
	public:
		typedef uint32_t result_type;

		explicit ChaCha(uint64_t seedval=0, uint64_t stream = 0);
		template<class Sseq> explicit ChaCha(Sseq& seq);

		void seed(uint64_t seedval, uint64_t stream = 0);
		template<class Sseq> void seed(Sseq& seq);

		uint32_t operator()();
		void discard(unsigned long long n);

		template<size_t R_> friend bool operator==(const ChaCha<R_>& lhs, const ChaCha<R_>& rhs);
		template<size_t R_> friend bool operator!=(const ChaCha<R_>& lhs, const ChaCha<R_>& rhs);

		template<typename CharT, typename Traits>
		friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ChaCha<R>& rng);

		template<typename CharT, typename Traits>
		friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, ChaCha<R>& rng);

		static constexpr uint32_t min() { return std::numeric_limits<uint32_t>::min(); }
		static constexpr uint32_t max() { return std::numeric_limits<uint32_t>::max(); }

	private:
		void generate_block();
		void chacha_core();

		alignas(16) uint32_t block[16];
		uint32_t keysetup[8];
		uint64_t ctr;
	};


	template<size_t R>
	inline ChaCha<R>::ChaCha(uint64_t seedval, uint64_t stream) {
		seed(seedval, stream);
	}

	template<size_t R>
	template<class Sseq>
	inline ChaCha<R>::ChaCha(Sseq& seq) {
		seed(seq);
	}

	template<size_t R>
	inline void ChaCha<R>::seed(uint64_t seedval, uint64_t stream) {
		ctr = 0;
		keysetup[0] = seedval & 0xffffffffu;
		keysetup[1] = seedval >> 32;
		keysetup[2] = keysetup[3] = 0xdeadbeef;      // Could use 128-bit seed.
		keysetup[4] = stream & 0xffffffffu;
		keysetup[5] = stream >> 32;
		keysetup[6] = keysetup[7] = 0xdeadbeef;      // Could use 128-bit stream.
	}

	template<size_t R>
	template<class Sseq>
	inline void ChaCha<R>::seed(Sseq& seq) {
		ctr = 0;
		seq.generate(keysetup, keysetup + 8);
	}


	template<size_t R>
	inline uint32_t ChaCha<R>::operator()() {
		int idx = ctr % 16;
		if (idx == 0) generate_block();
		++ctr;

		return block[idx];
	}

	template<size_t R>
	inline void ChaCha<R>::discard(unsigned long long n) {
		int idx = ctr % 16;
		ctr += n;
		if (idx + n >= 16 && ctr % 16 != 0) generate_block();
	}

	template<size_t R>
	inline void ChaCha<R>::generate_block() {
		uint32_t constants[4] = { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 };

		uint32_t input[16];
		for (int i = 0; i < 4; ++i) input[i] = constants[i];
		for (int i = 0; i < 8; ++i) input[4 + i] = keysetup[i];
		input[12] = (ctr / 16) & 0xffffffffu;
		input[13] = (ctr / 16) >> 32;
		input[14] = input[15] = 0xdeadbeef; // Could use 128-bit counter.

		for (int i = 0; i < 16; ++i) block[i] = input[i];
		chacha_core();
		for (int i = 0; i < 16; ++i) block[i] += input[i];
	}

	template<size_t R>
	inline void ChaCha<R>::chacha_core() {
#define CHACHA_ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define CHACHA_QUARTERROUND(x, a, b, c, d) \
        x[a] = x[a] + x[b]; x[d] ^= x[a]; x[d] = CHACHA_ROTL32(x[d], 16); \
        x[c] = x[c] + x[d]; x[b] ^= x[c]; x[b] = CHACHA_ROTL32(x[b], 12); \
        x[a] = x[a] + x[b]; x[d] ^= x[a]; x[d] = CHACHA_ROTL32(x[d],  8); \
        x[c] = x[c] + x[d]; x[b] ^= x[c]; x[b] = CHACHA_ROTL32(x[b],  7)

		for (int i = 0; i < R; i += 2) {
			CHACHA_QUARTERROUND(block, 0, 4, 8, 12);
			CHACHA_QUARTERROUND(block, 1, 5, 9, 13);
			CHACHA_QUARTERROUND(block, 2, 6, 10, 14);
			CHACHA_QUARTERROUND(block, 3, 7, 11, 15);
			CHACHA_QUARTERROUND(block, 0, 5, 10, 15);
			CHACHA_QUARTERROUND(block, 1, 6, 11, 12);
			CHACHA_QUARTERROUND(block, 2, 7, 8, 13);
			CHACHA_QUARTERROUND(block, 3, 4, 9, 14);
		}

#undef CHACHA_QUARTERROUND
#undef CHACHA_ROTL32
	}
}

template<typename Rng>
void testWrapper(benchmark::State& state) {
    constexpr uint32_t bitCount = 1 << 20;
    using T = typename Rng::result_type;
    constexpr uint32_t bits = sizeof(T)*8;
    static_assert(bitCount % bits == 0);
    constexpr uint32_t count = bitCount / bits;

    for(auto _ : state) {
        Rng rng;
        T sum = 0;
        for(uint32_t idx=0; idx<count; ++idx)
            sum += rng();
        benchmark::DoNotOptimize(sum);
    }
}

#define URNG(NAME, RNG) void bench_##NAME(benchmark::State& state) { testWrapper<RNG>(state); } BENCHMARK(bench_##NAME)

URNG(std_knuth_b, std::knuth_b);
URNG(std_minstd_rand0, std::minstd_rand0);
URNG(std_minstd_rand, std::minstd_rand);
URNG(std_mt19937, std::mt19937);
URNG(std_mt19937_64, std::mt19937_64);
URNG(PCG_XSH_RR_64_32_LCG, pcg32);
URNG(PCG_XSL_RR_128_64_LCG, pcg64);
URNG(PCG_XSL_RR_128_64_MCG, pcg64_fast);
URNG(std_ranlux24, std::ranlux24);
URNG(std_ranlux48, std::ranlux48);
URNG(Xoroshiro128pp, Xoroshiro128pp);
URNG(Xoshiro256pp, Xoshiro256pp);
URNG(ISAAC, ISAAC);
URNG(ChaCha20, ChaCha<20>);

BENCHMARK_MAIN();
