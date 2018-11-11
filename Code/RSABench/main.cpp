
#include "rsa_math__big_uint.h"

#include <benchmark/benchmark.h>

#include <random>

static void BM_RSA_division(benchmark::State& state)
{
	auto rng = std::mt19937_64();
	auto a_dist = std::uniform_int_distribution<std::uint16_t>(0u, rsa::math::meta::max<std::uint16_t>());

	auto a = rsa::math::big_uint_16();
	auto a_blocks = std::size_t(state.range(0));
	a.data().resize(a_blocks);

	for (auto i = 0u; i != a_blocks; ++i)
		a.data()[i] = a_dist(rng);

	auto b_dist = std::uniform_int_distribution<std::uint16_t>(0u, rsa::math::meta::max<std::uint16_t>());
	auto b = rsa::math::big_uint_16(b_dist(rng));

	for (auto _ : state)
	{
		a / b;
	}
}

BENCHMARK(BM_RSA_division)->Range(8u, 16384u)->Complexity()->Unit(benchmark::kMicrosecond);


#include "rsa_math__primes.h"

static void BM_RSA_generate_prime(benchmark::State& state)
{
	auto rng = std::mt19937_64();

	for (auto _ : state)
		benchmark::DoNotOptimize(rsa::math::generate_prime<std::uint32_t>(rng, state.range(0)));
}

BENCHMARK(BM_RSA_generate_prime)->RangeMultiplier(2u)->Range(2u, 64u)->Complexity()->Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();
