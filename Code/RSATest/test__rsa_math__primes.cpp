
#include "rsa_math__primes.h"

#include <gtest/gtest.h>

#include <random>

namespace test
{

	TEST(Test_RSA, math_primes_generate_random_bits)
	{
		auto rng = std::mt19937_64();
		auto n = rsa::math::generate_random_bits<std::uint32_t>(rng, 4096u);
		EXPECT_FALSE(n.data().empty());
		EXPECT_TRUE(std::all_of(n.data().begin(), n.data().end(), [] (std::uint32_t block) { return block != 0u; }));
	}

	// TODO: make sure number is less than the max for the bit length?

	TEST(Test_RSA, math_primes_is_prime_n)
	{
		EXPECT_EQ(rsa::math::is_prime(0), false);
		EXPECT_EQ(rsa::math::is_prime(1), true);
		EXPECT_EQ(rsa::math::is_prime(2), true);
		EXPECT_EQ(rsa::math::is_prime(3), true);
		EXPECT_EQ(rsa::math::is_prime(4), false);
		EXPECT_EQ(rsa::math::is_prime(5), true);
		EXPECT_EQ(rsa::math::is_prime(6), false);
		EXPECT_EQ(rsa::math::is_prime(7), true);
		EXPECT_EQ(rsa::math::is_prime(8), false);
		EXPECT_EQ(rsa::math::is_prime(9), false);
		EXPECT_EQ(rsa::math::is_prime(10), false);
		EXPECT_EQ(rsa::math::is_prime(11), true);
		EXPECT_EQ(rsa::math::is_prime(57047), true);
		EXPECT_EQ(rsa::math::is_prime(57059), true);
		EXPECT_EQ(rsa::math::is_prime(57061), false);
		EXPECT_EQ(rsa::math::is_prime(57073), true);
		EXPECT_EQ(rsa::math::is_prime(57079), false);
	}

	TEST(Test_RSA, math_primes_is_prime_big_uint)
	{
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_8(0u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_64(1u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(2u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_16(3u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(4u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_64(5u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(6u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(7u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_8(8u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(9u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_16(10u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(11u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_64(57047u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(57059u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(57061u)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_64(57073u)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::big_uint_32(57079u)), false);
	}

	TEST(Test_RSA, math_primes_is_prime_rm)
	{
		auto k = 20u;
		auto rng = std::mt19937_64();
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_16(0u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_64(1u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(2u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_16(3u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(4u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_64(5u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(6u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(7u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(8u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(9u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_16(10u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(11u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_64(57047u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(57059u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(57061u), rng, k), false);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_64(57073u), rng, k), true);
		EXPECT_EQ(rsa::math::is_prime_rm(rsa::math::big_uint_32(57079u), rng, k), false);

		{
			auto n = rsa::math::generate_random_bits<std::uint64_t>(rng, 4096u); // big number!
			EXPECT_NO_THROW(rsa::math::is_prime_rm(n, rng, k));
		}
	}

	TEST(Test_RSA, math_primes_generate_prime__ReturnsPrime)
	{
		auto k = 20u;
		auto rng = std::mt19937_64();
		EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime<std::uint32_t>(rng, 16u), rng, k));
		EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime<std::uint32_t>(rng, 24u), rng, k));
	}

	TEST(Test_RSA, math_primes_generate_prime_rm__ReturnsPrime)
	{
		auto k = 20u;
		auto rng = std::mt19937_64();
		EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 16u, k), rng, k));
		std::cout << "16" << std::endl;
		EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 24u, k), rng, k));
		std::cout << "24" << std::endl;
		// too slow in debug, but ok in release:
		//EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 32u, k), rng, k));
		//std::cout << "32" << std::endl;
		//EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 64u, k), rng, k));
		//std::cout << "64" << std::endl;
		//EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 128u, k), rng, k));
		//std::cout << "128" << std::endl;
		//EXPECT_TRUE(rsa::math::is_prime_rm(rsa::math::generate_prime_rm<std::uint64_t>(rng, 256u, k), rng, k));
		//std::cout << "256" << std::endl;
		// larger numbers are slow in release too... :(
	}

} // test
