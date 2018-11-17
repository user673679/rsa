
#include "rsa_math__big_uint.h"
#include "rsa_math__primes.h"

#include <benchmark/benchmark.h>

#include <random>

//static void BM_RSA_division(benchmark::State& state)
//{
//	auto rng = std::mt19937_64();
//	auto a_dist = std::uniform_int_distribution<std::uint16_t>(0u, rsa::math::meta::max<std::uint16_t>());
//
//	auto a = rsa::math::big_uint_16();
//	auto a_blocks = std::size_t(state.range(0));
//	a.data().resize(a_blocks);
//
//	for (auto i = 0u; i != a_blocks; ++i)
//		a.data()[i] = a_dist(rng);
//
//	auto b_dist = std::uniform_int_distribution<std::uint16_t>(0u, rsa::math::meta::max<std::uint16_t>());
//	auto b = rsa::math::big_uint_16(b_dist(rng));
//
//	for (auto _ : state)
//	{
//		a / b;
//	}
//}
//
//BENCHMARK(BM_RSA_division)->Range(8u, 16384u)->Complexity()->Unit(benchmark::kMicrosecond);

//static void BM_RSA_generate_prime(benchmark::State& state)
//{
//	auto rng = std::mt19937_64();
//
//	for (auto _ : state)
//		benchmark::DoNotOptimize(rsa::math::generate_prime<std::uint32_t>(rng, state.range(0)));
//}
//
//BENCHMARK(BM_RSA_generate_prime)->RangeMultiplier(2u)->Range(2u, 64u)->Complexity()->Unit(benchmark::kMicrosecond);
//
//static void BM_RSA_is_prime_rm_32(benchmark::State& state)
//{
//	auto n_rng = std::mt19937_64();
//	auto ns = std::vector<rsa::math::big_uint_32>();
//	for (auto i = 0u; i != 50u; ++i)
//		ns.push_back(rsa::math::generate_random_bits<std::uint32_t>(n_rng, std::size_t(state.range(0))));
//
//	auto k = 20u;
//
//	auto rng = std::mt19937_64();
//	for (auto _ : state)
//		for (auto const& n : ns)
//			benchmark::DoNotOptimize(rsa::math::is_prime_rm(n, rng, k));
//}
//
//BENCHMARK(BM_RSA_is_prime_rm_32)->RangeMultiplier(2u)->Range(2u, 2048u)->Complexity()->Unit(benchmark::kMicrosecond);
//
//static void BM_RSA_is_prime_rm_64(benchmark::State& state)
//{
//	auto n_rng = std::mt19937_64();
//	auto ns = std::vector<rsa::math::big_uint_64>();
//	for (auto i = 0u; i != 50u; ++i)
//		ns.push_back(rsa::math::generate_random_bits<std::uint64_t>(n_rng, std::size_t(state.range(0))));
//
//	auto k = 20u;
//
//	auto rng = std::mt19937_64();
//	for (auto _ : state)
//		for (auto const& n : ns)
//			benchmark::DoNotOptimize(rsa::math::is_prime_rm(n, rng, k));
//}
//
//BENCHMARK(BM_RSA_is_prime_rm_64)->RangeMultiplier(2u)->Range(2u, 2048u)->Complexity()->Unit(benchmark::kMicrosecond);

//static void BM_RSA_is_prime(benchmark::State& state)
//{
//	auto n_rng = std::mt19937_64();
//	auto ns = std::vector<rsa::math::big_uint_32>();
//	for (auto i = 0u; i != 50u; ++i)
//		ns.push_back(rsa::math::generate_random_bits<std::uint32_t>(n_rng, state.range(0)));
//
//	for (auto _ : state)
//		for (auto const& n : ns)
//			benchmark::DoNotOptimize(rsa::math::is_prime(n));
//}
//
//BENCHMARK(BM_RSA_is_prime)->RangeMultiplier(2u)->Range(2u, 48u)->Complexity()->Unit(benchmark::kMicrosecond);

static void BM_RSA_generate_prime_rm(benchmark::State& state)
{
	auto rng = std::mt19937_64(5489u);
	auto k = 20u;

	for (auto _ : state)
		benchmark::DoNotOptimize(rsa::math::generate_prime_rm<std::uint32_t>(rng, std::size_t(state.range(0)), k));
}

BENCHMARK(BM_RSA_generate_prime_rm)->RangeMultiplier(2u)->Range(16u, 1024u)->Complexity()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
