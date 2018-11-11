
#include "rsa_math__big_uint.h"
#include "test_utils__uint_max.h"

#include <gtest/gtest.h>

namespace test
{

#pragma region old

	//TEST(Test_RSA, math_is_prime_n__IsCorrect)
	//{
	//	EXPECT_EQ(rsa::math::is_prime(0), false);
	//	EXPECT_EQ(rsa::math::is_prime(1), true);
	//	EXPECT_EQ(rsa::math::is_prime(2), true);
	//	EXPECT_EQ(rsa::math::is_prime(3), true);
	//	EXPECT_EQ(rsa::math::is_prime(4), false);
	//	EXPECT_EQ(rsa::math::is_prime(5), true);
	//	EXPECT_EQ(rsa::math::is_prime(6), false);
	//	EXPECT_EQ(rsa::math::is_prime(7), true);
	//	EXPECT_EQ(rsa::math::is_prime(8), false);
	//	EXPECT_EQ(rsa::math::is_prime(9), false);
	//	EXPECT_EQ(rsa::math::is_prime(10), false);
	//	EXPECT_EQ(rsa::math::is_prime(11), true);
	//	EXPECT_EQ(rsa::math::is_prime(57047), true);
	//	EXPECT_EQ(rsa::math::is_prime(57059), true);
	//	EXPECT_EQ(rsa::math::is_prime(57061), false);
	//	EXPECT_EQ(rsa::math::is_prime(57073), true);
	//	EXPECT_EQ(rsa::math::is_prime(57079), false);
	//}

	//TEST(Test_RSA, math_is_prime_bits__IsCorrect)
	//{
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(0)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(1)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(2)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(3)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(4)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(5)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(6)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(7)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(8)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(9)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(10)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(11)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57047)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57059)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57061)), false);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57073)), true);
	//	EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57079)), false);
	//}

	//TEST(Test_RSA, generate_random_bits__SizeOfResultIsCorrect)
	//{
	//	auto rng = std::mt19937_64(std::random_device()());
	//	EXPECT_EQ(rsa::generate_random_bits(rng, 0).size(), 0);
	//	EXPECT_EQ(rsa::generate_random_bits(rng, 1).size(), 1);
	//	EXPECT_EQ(rsa::generate_random_bits(rng, rsa::MAX_RANDOM_BITS).size(), rsa::MAX_RANDOM_BITS);
	//}

	//TEST(Test_RSA, generate_random_bits__ThrowsIfBitsOutOfRange)
	//{
	//	auto rng = std::mt19937_64(std::random_device()());
	//	EXPECT_THROW(rsa::generate_random_bits(rng, rsa::MAX_RANDOM_BITS + 1), std::invalid_argument);
	//	EXPECT_THROW(rsa::generate_random_bits(rng, std::numeric_limits<std::size_t>::max()), std::invalid_argument);
	//}

	//TEST(Test_RSA, generate_random_bits__ContainsZerosAndOnes)
	//{
	//	auto rng = std::mt19937_64(std::random_device()());
	//	auto data = rsa::generate_random_bits(rng, 2048);
	//	EXPECT_FALSE(std::all_of(data.begin(), data.end(), [] (bool b) { return b; }));
	//	EXPECT_FALSE(std::all_of(data.begin(), data.end(), [] (bool b) { return !b; }));
	//}

	//TEST(Test_RSA, generate_prime__ThrowsIfBitsOutOfRange)
	//{
	//	EXPECT_THROW(rsa::generate_prime(0), std::invalid_argument);
	//	EXPECT_THROW(rsa::generate_prime(rsa::MAX_PRIME_BITS + 1), std::invalid_argument);
	//}

	//TEST(Test_RSA, generate_prime__ReturnsPrime)
	//{
	//	EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(16))));
	//	EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(24))));
	//	// too slow... TT
	//	//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(32))));
	//	//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(256))));
	//	//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(512))));
	//	//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(1024))));
	//}

#pragma endregion

#pragma region constructors

	TEST(Test_RSA, math_big_uint_default_constructor__DataIsEmptySameAsZero)
	{
		{
			auto n = rsa::math::big_uint_32();
			EXPECT_TRUE(n.data().empty());
			EXPECT_EQ(n, rsa::math::big_uint_32(std::uint32_t{ 0 }));
		}
	}

	TEST(Test_RSA, math_big_uint_numeric_constructor__WorksWithLargerAndSmallerNumbers)
	{
		auto a = rsa::math::big_uint_32(std::uint16_t{ 37 });
		EXPECT_EQ(a.data().size(), 1u);
		EXPECT_EQ(a.data()[0], std::uint32_t{ 37 });

		auto b = rsa::math::big_uint_32(std::uint64_t{ 37 });
		EXPECT_EQ(b.data().size(), 1u);
		EXPECT_EQ(b.data()[0], std::uint32_t{ 37 });

		EXPECT_EQ(a, b);
	}

	TEST(Test_RSA, math_big_uint_numeric_constructor__GivesCorrectBitPattern)
	{
		{
			auto n = rsa::math::big_uint_32(1u);
			EXPECT_TRUE(n.data().size() == 1);
			EXPECT_EQ(n.data()[0], std::uint32_t{ 1 });
		}
		{
			auto n = rsa::math::big_uint_32(53u);
			EXPECT_TRUE(n.data().size() == 1);
			EXPECT_EQ(n.data()[0], std::uint32_t{ 53 });
		}
		{
			auto n = rsa::math::big_uint_32(utils::uint32_max);
			EXPECT_EQ(n.data().size(), 1);
			EXPECT_EQ(n.data()[0], utils::uint32_max);
		}
		{
			auto n = rsa::math::big_uint_32(std::uint64_t{ utils::uint32_max } +1u);
			EXPECT_TRUE(n.data().size() == 2);
			EXPECT_EQ(n.data()[0], 0u);
			EXPECT_EQ(n.data()[1], 1u);
		}
		{
			auto n = rsa::math::big_uint_32(utils::uint64_max);
			EXPECT_TRUE(n.data().size() == 2);
			EXPECT_EQ(n.data()[0], utils::uint32_max);
			EXPECT_EQ(n.data()[1], utils::uint32_max);
		}
		{
			auto n = rsa::math::big_uint_8(utils::uint64_max);
			EXPECT_TRUE(n.data().size() == 8);
			for (auto i = std::size_t{ 0 }; i != 8; ++i)
				EXPECT_EQ(n.data()[i], utils::uint8_max);
		}
	}

#pragma endregion

#pragma region assignment

	// not testing copy / move (default)

	TEST(Test_RSA, math_big_uint_value_assign__Works)
	{
		auto max = utils::uint64_max;
		auto n = rsa::math::big_uint_32(5u);
		n = max;

		EXPECT_EQ(n, max);
		EXPECT_EQ(n, rsa::math::big_uint_32(max));
		EXPECT_EQ(n.to_uint<std::uint64_t>(), max);
	}

#pragma endregion

#pragma region general

	TEST(Test_RSA, math_big_uint_to_uint__UintsSmallerThanBlockWontCompile)
	{
		//EXPECT_THROW(rsa::math::big_uint_16(1u).to_uint<std::uint8_t>(), std::range_error);

		//EXPECT_THROW(rsa::math::big_uint_32(1u).to_uint<std::uint8_t>(), std::range_error);
		//EXPECT_THROW(rsa::math::big_uint_32(1u).to_uint<std::uint16_t>(), std::range_error);

		//EXPECT_THROW(rsa::math::big_uint_64(1u).to_uint<std::uint8_t>(), std::range_error);
		//EXPECT_THROW(rsa::math::big_uint_64(1u).to_uint<std::uint16_t>(), std::range_error);
		//EXPECT_THROW(rsa::math::big_uint_64(1u).to_uint<std::uint32_t>(), std::range_error);
	}

	TEST(Test_RSA, math_big_uint_to_uint__ThrowsForOutOfRangeTypes)
	{
		const auto max = utils::uint32_max;

		EXPECT_THROW(rsa::math::big_uint_8(max).to_uint<std::uint8_t>(), std::range_error);
		EXPECT_THROW((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint32_t>(), std::range_error);
	}

	TEST(Test_RSA, math_big_uint_to_uint__ReturnsCorrectValueWhenInRange)
	{
		const auto max = utils::uint32_max;

		EXPECT_EQ(rsa::math::big_uint_8(max).to_uint<std::uint32_t>(), max);
		EXPECT_EQ((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint64_t>(), std::uint64_t{ max } +1u);
	}

	TEST(Test_RSA, math_big_uint_get_bit)
	{
		{
			auto zero = rsa::math::big_uint_8(0u);
			EXPECT_EQ(zero.get_bit(0u), false);
			EXPECT_EQ(zero.get_bit(123585u), false);
		}
		{
			auto one = rsa::math::big_uint_8(1u);
			EXPECT_EQ(one.get_bit(0u), true);
			EXPECT_EQ(one.get_bit(123585u), false);
		}
		{
			auto n = rsa::math::big_uint_16(utils::uint32_max);
			EXPECT_EQ(n.get_bit(0u), true);
			EXPECT_EQ(n.get_bit(31u), true);
			EXPECT_EQ(n.get_bit(32u), false);
		}
		{
			auto n = rsa::math::big_uint_8(21u);
			EXPECT_EQ(n.get_bit(0u), true);
			EXPECT_EQ(n.get_bit(1u), false);
			EXPECT_EQ(n.get_bit(2u), true);
			EXPECT_EQ(n.get_bit(3u), false);
			EXPECT_EQ(n.get_bit(4u), true);
			EXPECT_EQ(n.get_bit(5u), false);
		}
	}

	TEST(Test_RSA, math_big_uint_set_bit)
	{
		{
			auto n = rsa::math::big_uint_8(0u);
			n.set_bit(0u, true);
			EXPECT_EQ(n, 1u);
		}
		{
			auto n = rsa::math::big_uint_8(0u);
			n.set_bit(0u, true);
			n.set_bit(2u, true);
			n.set_bit(4u, true);
			EXPECT_EQ(n, 21u);
		}
		{
			auto n = rsa::math::big_uint_8(0u);
			for (auto i = 0u; i != 32u; ++i)
				n.set_bit(i, true);
			EXPECT_EQ(n, utils::uint32_max);
		}
	}

	TEST(Test_RSA, math_big_uint_flip)
	{
		{
			auto n = rsa::math::big_uint_8(0u);
			n.flip_bit(0u);
			EXPECT_EQ(n, 1u);
		}
		{
			auto n = rsa::math::big_uint_8(0u);
			n.flip_bit(0u);
			n.flip_bit(2u);
			n.flip_bit(4u);
			EXPECT_EQ(n, 21u);
		}
		{
			auto n = rsa::math::big_uint_16(21u);

			for (auto i = 0u; i != 5u; ++i)
				n.flip_bit(i);

			EXPECT_EQ(n, 10u);
		}
	}

	TEST(Test_RSA, math_big_uint_find_most_significant_bit)
	{
		{
			auto n = rsa::math::big_uint_64(0u);
			EXPECT_THROW(n.get_most_significant_bit(), std::logic_error);
		}
		{
			auto n = rsa::math::big_uint_64(1u);
			EXPECT_EQ(n.get_most_significant_bit(), 0u);
		}
		{
			auto n = rsa::math::big_uint_64(21u);
			EXPECT_EQ(n.get_most_significant_bit(), 4u);
		}
	}

#pragma endregion

#pragma region bitwise operators

	TEST(Test_RSA, math_big_uint_bit_and_assign__IsCorrect)
	{
		{
			auto zero = rsa::math::big_uint_32(0u);
			EXPECT_TRUE((zero &= zero).is_zero());
		}
		{
			auto zero = rsa::math::big_uint_32(0u);
			auto something = rsa::math::big_uint_32(3476u);
			EXPECT_EQ(zero &= something, 0u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			auto b = rsa::math::big_uint_8(19787u);
			EXPECT_EQ(a &= b, std::uint32_t{ 12395787u } & 19787u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			EXPECT_EQ(a &= a, std::uint32_t{ 12395787u });
		}
	}

	TEST(Test_RSA, math_big_uint_bit_and__IsCorrect)
	{
		auto a = rsa::math::big_uint_8(123987u);
		EXPECT_EQ(a & 19787u, std::uint32_t{ 123987u } & 19787u);
	}

	TEST(Test_RSA, math_big_uint_bit_or_assign__IsCorrect)
	{
		{
			auto zero = rsa::math::big_uint_32(0u);
			EXPECT_TRUE((zero |= zero).is_zero());
		}
		{
			auto zero = rsa::math::big_uint_32(0u);
			auto something = rsa::math::big_uint_32(3476u);
			EXPECT_EQ(zero |= something, something);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			auto b = rsa::math::big_uint_8(19787u);
			EXPECT_EQ(a |= b, std::uint32_t{ 12395787u } | 19787u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			EXPECT_EQ(a |= a, std::uint32_t{ 12395787u });
		}
	}

	TEST(Test_RSA, math_big_uint_bit_or__IsCorrect)
	{
		auto a = rsa::math::big_uint_8(127u);
		EXPECT_EQ(a | 19787u, std::uint32_t{ 127u } | 19787u);
	}

	TEST(Test_RSA, math_big_uint_bit_xor_assign__IsCorrect)
	{
		{
			auto zero = rsa::math::big_uint_32(0u);
			EXPECT_TRUE((zero ^= zero).is_zero());
		}
		{
			auto zero = rsa::math::big_uint_32(0u);
			auto something = rsa::math::big_uint_32(3476u);
			EXPECT_EQ(zero ^= something, something);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			auto b = rsa::math::big_uint_8(19787u);
			EXPECT_EQ(a ^= b, std::uint32_t{ 12395787u } ^ 19787u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			EXPECT_TRUE((a ^= a).is_zero());
		}
	}

	TEST(Test_RSA, math_big_uint_bit_xor__IsCorrect)
	{
		auto a = rsa::math::big_uint_8(127u);
		EXPECT_EQ(a ^ 19787u, std::uint32_t{ 127u } ^ 19787u);
	}

	TEST(Test_RSA, math_big_uint_lshift_assign__IsCorrect)
	{
		// shifting by zero does nothing
		{
			auto n = rsa::math::big_uint_8(utils::uint8_max);
			EXPECT_EQ(n <<= 0u, utils::uint8_max);
		}
		// shifting zero does nothing
		{
			auto zero = rsa::math::big_uint_16();
			EXPECT_TRUE((zero <<= 23u).is_zero());
		}
		// shifting by whole block only
		{
			auto n = rsa::math::big_uint_8(utils::uint8_max);
			EXPECT_EQ(n <<= 16u, std::uint32_t{ utils::uint8_max } << 16u);
			EXPECT_EQ(n.data().size(), 3u);
		}
		// shifting within block only
		{
			auto n = rsa::math::big_uint_32(utils::uint16_max);
			EXPECT_EQ(n <<= 16u, utils::uint32_max - utils::uint16_max);
			EXPECT_EQ(n.data().size(), 1u);
		}
		// shifting by whole and partial block
		{
			auto n = rsa::math::big_uint_8(1u);
			EXPECT_EQ(n <<= (7u * 8u + 4u), std::uint64_t{ 1u } << (7u * 8u + 4u));
			EXPECT_EQ(n.data().size(), 8u);
		}
		{
			auto n = rsa::math::big_uint_8(12u);
			EXPECT_EQ(n <<= 1u, 12u << 1u);
		}
	}

	TEST(Test_RSA, math_big_uint_lshift__IsCorrect)
	{
		auto n = rsa::math::big_uint_8(1u);
		EXPECT_EQ((n << 1u), (1u << 1u));
	}

	TEST(Test_RSA, math_big_uint_rshift_assign__IsCorrect)
	{
		// shifting by zero does nothing
		{
			auto n = rsa::math::big_uint_8(utils::uint8_max);
			EXPECT_EQ(n >>= 0u, utils::uint8_max);
		}
		// shifting zero does nothing
		{
			auto zero = rsa::math::big_uint_16();
			EXPECT_TRUE((zero >>= 23u).is_zero());
		}
		// shifting by whole block only
		{
			auto n = rsa::math::big_uint_8(utils::uint32_max);
			EXPECT_EQ(n >>= 16u, utils::uint16_max);
			EXPECT_EQ(n.data().size(), 2u);
		}
		// shifting within block only
		{
			auto n = rsa::math::big_uint_32(utils::uint16_max);
			EXPECT_EQ(n >>= 8u, utils::uint8_max);
			EXPECT_EQ(n.data().size(), 1u);
		}
		// shifting by whole and partial block
		{
			auto n = rsa::math::big_uint_8(utils::uint64_max);
			EXPECT_EQ(n >>= (7u * 8u + 4u), std::uint32_t{ utils::uint8_max >> 4u });
			EXPECT_EQ(n.data().size(), 1u);
		}
	}

	TEST(Test_RSA, math_big_uint_rshift__IsCorrect)
	{
		auto n = rsa::math::big_uint_8(1u);
		EXPECT_TRUE((n >> 1u).is_zero());
	}

#pragma endregion

#pragma region math operators

	TEST(Test_RSA, math_addassign__BigUintIsCorrect)
	{
		// a (two blocks), b (two blocks), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +max16);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +max16);
		}
		// a (two blocks), b (two blocks), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max16 / 2u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 / 2u + max16 / 2u });
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 / 2u + max16 / 2u });
		}
		// a (two blocks), b (one block), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(1u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +1u);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +1u);
		}
		// a (one block), b (two blocks), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +1u);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +1u);
		}
		// a (two blocks), b (one block), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max8);
			a += b;
			EXPECT_EQ(a, (max16 / 2u) + max8);
			EXPECT_EQ(a.to_uint<std::uint16_t>(), (max16 / 2u) + max8);
		}
		// a (one block), b (two blocks), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_8(max8);
			auto b = rsa::math::big_uint_8(max16 / 2u);
			a += b;
			EXPECT_EQ(a, (max16 / 2u) + max8);
			EXPECT_EQ(a.to_uint<std::uint16_t>(), (max16 / 2u) + max8);
		}
	}

	TEST(Test_RSA, math_addassign__ValueIsCorrect)
	{
		// assigning with zero
		{
			auto n = rsa::math::big_uint_32(0u);
			n += rsa::math::big_uint_32(0u);
			EXPECT_EQ(n, 0u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), 0u);
		}
		{
			auto max = utils::uint32_max;
			auto n = rsa::math::big_uint_32(0u);
			n += max;
			EXPECT_EQ(n, max);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), max);
		}
		{
			auto max = utils::uint32_max;
			auto n = rsa::math::big_uint_32(max);
			n += 0u;
			EXPECT_EQ(n, max);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), max);
		}
		// assigning inside block
		{
			auto max = utils::uint8_max;
			auto n = rsa::math::big_uint_16(max);
			n += max;
			EXPECT_EQ(n, std::uint32_t{ max } +max);
			EXPECT_EQ(n.to_uint<std::uint16_t>(), std::uint16_t{ max } +max);
		}
		// assigning outside block
		{
			auto max = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max);
			n += 1u;
			EXPECT_EQ(n, std::uint16_t{ max } +1u);
			EXPECT_EQ(n.to_uint<std::uint16_t>(), std::uint16_t{ max } +1u);
		}
		{
			auto max16 = utils::uint16_max;
			auto max32 = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max16);
			n += max32;
			EXPECT_EQ(n, std::uint64_t{ max32 } +max16);
			EXPECT_EQ(n.to_uint<std::uint64_t>(), std::uint64_t{ max32 } +max16);
		}
	}

	TEST(Test_RSA, math_add__IsCorrect)
	{
		{
			auto n = rsa::math::big_uint_32(23u) + rsa::math::big_uint_32(78u);
			EXPECT_EQ(n, 23u + 78u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), 23u + 78u);
		}
		{
			auto max8 = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max8) + 2u;
			EXPECT_EQ(n, std::uint32_t{ max8 } +2u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } +2u);
		}
		{
			auto max8 = utils::uint8_max;
			auto n = max8 + rsa::math::big_uint_8(1u);
			EXPECT_EQ(n, std::uint32_t{ max8 } +1u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } +1u);
		}
	}

	TEST(Test_RSA, math_subassign__SubtractingLargerNumberThrows)
	{
		EXPECT_THROW(rsa::math::big_uint_32(0u) -= rsa::math::big_uint_32(1u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_32(5u) -= rsa::math::big_uint_32(6u), std::invalid_argument);
	}

	TEST(Test_RSA, math_subassign__BigUintIsCorrect)
	{
		{
			auto zero = rsa::math::big_uint_32(0u);
			EXPECT_EQ(zero -= zero, zero);
		}
		{
			auto max64 = utils::uint64_max;
			auto n = rsa::math::big_uint_8(max64);
			EXPECT_EQ(n -= n, 0u);
			EXPECT_TRUE(n.is_zero());
		}
		{
			auto max16 = utils::uint16_max;
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_8(std::uint32_t{ max16 } + 1u);
			auto b = rsa::math::big_uint_8((std::uint32_t{ max16 } - max8) + 1u);
			a -= b;
			EXPECT_EQ(a, max8);
			EXPECT_EQ(a.data().size(), 1u);
		}
	}

	TEST(Test_RSA, math_subassign__ValueIsCorrect)
	{
		{
			auto max64 = utils::uint64_max;
			auto n = rsa::math::big_uint_8(max64);
			EXPECT_EQ(n -= 0u, n);
		}
		{
			auto max32 = utils::uint32_max;
			auto max64 = utils::uint64_max;
			auto n = rsa::math::big_uint_32(max64);
			EXPECT_EQ(n -= max32, max64 - max32);
			EXPECT_EQ(n.data()[0], 0u);
		}
	}

	TEST(Test_RSA, math_sub__IsCorrect)
	{
		{
			auto a = rsa::math::big_uint_32(2389u);
			auto b = rsa::math::big_uint_32(192847u);
			EXPECT_THROW(a - b, std::invalid_argument);
		}
		{
			auto a = rsa::math::big_uint_32(192847u);
			auto b = 2389u;
			EXPECT_EQ(a - b, (192847u - 2389u));
		}
		{
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_16(std::uint16_t{ max8 } + 3u);
			auto b = 3u;
			EXPECT_EQ(a - b, max8);
		}
	}

	TEST(Test_RSA, math_mulassign__BigUint)
	{
		// zeros
		{
			auto a = rsa::math::big_uint_8(0u);
			auto b = rsa::math::big_uint_8(0u);
			EXPECT_TRUE((a *= b).is_zero());
		}
		{
			auto a = rsa::math::big_uint_8(0u);
			auto b = rsa::math::big_uint_8(56u);
			EXPECT_TRUE((a *= b).is_zero());
		}
		{
			auto a = rsa::math::big_uint_8(45u);
			auto b = rsa::math::big_uint_8(0u);
			EXPECT_TRUE((a *= b).is_zero());
		}
		// ones
		{
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(1u);
			EXPECT_EQ((a *= b), 1u);
		}
		{
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(56u);
			EXPECT_EQ((a *= b), 56u);
		}
		{
			auto a = rsa::math::big_uint_8(45u);
			auto b = rsa::math::big_uint_8(1u);
			EXPECT_EQ((a *= b), 45u);
		}
		// within block
		{
			auto a = rsa::math::big_uint_8(12u);
			EXPECT_EQ(a *= a, 144u);
		}
		{
			auto a = rsa::math::big_uint_8(5u);
			auto b = rsa::math::big_uint_8(20u);
			EXPECT_EQ(a *= b, 100u);
		}
		// outside block
		{
			auto a = rsa::math::big_uint_8(2u);
			auto b = rsa::math::big_uint_8(128u);
			EXPECT_EQ(a *= b, 256u);
			EXPECT_EQ(a.data().size(), 2u);
		}
		{
			auto a = rsa::math::big_uint_8(43u);
			auto b = rsa::math::big_uint_8(24892368u);
			EXPECT_EQ(a *= b, 43u * 24892368u);
		}
	}

	TEST(Test_RSA, math_mulassign__Value)
	{
		auto a = rsa::math::big_uint_16(utils::uint8_max);
		EXPECT_EQ(a *= utils::uint8_max, std::uint32_t{ utils::uint8_max } *utils::uint8_max);
	}

	TEST(Test_RSA, math_mul__IsCorrect)
	{
		{
			auto a = rsa::math::big_uint_16(utils::uint8_max);
			auto b = rsa::math::big_uint_16(utils::uint16_max);
			EXPECT_EQ(a * b, std::uint32_t{ utils::uint16_max } *utils::uint8_max);
		}
		{
			auto a = rsa::math::big_uint_16(utils::uint8_max);
			EXPECT_EQ(a * utils::uint32_max, std::uint64_t{ utils::uint32_max } * utils::uint8_max);
		}
		{
			auto a = rsa::math::big_uint_16(utils::uint8_max);
			EXPECT_EQ(utils::uint32_max * a, std::uint64_t{ utils::uint32_max } * utils::uint8_max);
		}
	}

	TEST(Test_RSA, math_divassign__BigUintDivisionByZeroThrows)
	{
		EXPECT_THROW(rsa::math::big_uint_8(0u) /= rsa::math::big_uint_8(0u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_8(1u) /= rsa::math::big_uint_8(0u), std::invalid_argument);
	}

	TEST(Test_RSA, math_divassign__BigUint)
	{
		EXPECT_TRUE((rsa::math::big_uint_16(0u) /= rsa::math::big_uint_16(4u)).is_zero());
		EXPECT_EQ(rsa::math::big_uint_16(127564u) /= rsa::math::big_uint_16(127564u), rsa::math::big_uint_16(1u));
		EXPECT_TRUE((rsa::math::big_uint_8(utils::uint32_max) /= rsa::math::big_uint_8(std::uint64_t{ utils::uint32_max } +1u)).is_zero());

		{
			auto a = rsa::math::big_uint_64(8u);
			auto b = rsa::math::big_uint_64(2u);
			EXPECT_EQ(a /= b, 4u);
		}
		{
			auto a = rsa::math::big_uint_32(13u);
			auto b = rsa::math::big_uint_32(3u);
			EXPECT_EQ(a /= b, 4u);
		}
		{
			auto a = rsa::math::big_uint_8(utils::uint64_max);
			auto b = rsa::math::big_uint_8(3u);
			EXPECT_EQ(a /= b, utils::uint64_max / 3u);
		}
		{
			auto a = rsa::math::big_uint_16(5u);
			auto b = rsa::math::big_uint_16(3u);
			EXPECT_EQ(a /= b, 1u);
		}
	}

	TEST(Test_RSA, math_div__IsCorrect)
	{
		EXPECT_EQ(rsa::math::big_uint_32(64u) / rsa::math::big_uint_32(8u), 8u);
		EXPECT_EQ(rsa::math::big_uint_8(257u) / 255u, 1u);
		EXPECT_EQ(908734u / rsa::math::big_uint_16(utils::uint64_max), 0u);
	}

	TEST(Test_RSA, math_modassign__BigUintModulusByZeroThrows)
	{
		EXPECT_THROW(rsa::math::big_uint_8(0u) %= rsa::math::big_uint_8(0u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_8(1u) %= rsa::math::big_uint_8(0u), std::invalid_argument);
	}

	TEST(Test_RSA, math_modassign__BigUint)
	{
		EXPECT_TRUE((rsa::math::big_uint_16(0u) %= rsa::math::big_uint_16(4u)).is_zero());
		EXPECT_EQ(rsa::math::big_uint_16(127564u) %= rsa::math::big_uint_16(127564u), rsa::math::big_uint_16(0u));
		EXPECT_EQ(rsa::math::big_uint_8(utils::uint32_max) %= rsa::math::big_uint_8(std::uint64_t{ utils::uint32_max } + 1u), utils::uint32_max);

		{
			auto a = rsa::math::big_uint_64(8u);
			auto b = rsa::math::big_uint_64(2u);
			EXPECT_EQ(a %= b, 0u);
		}
		{
			auto a = rsa::math::big_uint_32(13u);
			auto b = rsa::math::big_uint_32(3u);
			EXPECT_EQ(a %= b, 1u);
		}
		{
			auto a = rsa::math::big_uint_8(utils::uint64_max);
			auto b = rsa::math::big_uint_8(3u);
			EXPECT_EQ(a %= b, utils::uint64_max % 3u);
		}
		{
			auto a = rsa::math::big_uint_16(5u);
			auto b = rsa::math::big_uint_16(3u);
			EXPECT_EQ(a %= b, 2u);
		}
	}

	TEST(Test_RSA, math_mod__IsCorrect)
	{
		EXPECT_EQ(rsa::math::big_uint_32(64u) % rsa::math::big_uint_32(8u), 0u);
		EXPECT_EQ(rsa::math::big_uint_8(257u) % 255u, 2u);
		EXPECT_EQ(908734u % rsa::math::big_uint_16(utils::uint64_max), 908734u);
	}

	TEST(Test_RSA, math_increment)
	{
		{
			auto n = rsa::math::big_uint_16(23u);
			EXPECT_EQ(++n, 24u);
			EXPECT_EQ(n, 24u);
		}
		{
			auto n = rsa::math::big_uint_16(23u);
			EXPECT_EQ(n++, 23u);
			EXPECT_EQ(n, 24u);
		}
	}

	TEST(Test_RSA, math_decrement)
	{
		{
			auto n = rsa::math::big_uint_16(23u);
			EXPECT_EQ(--n, 22u);
			EXPECT_EQ(n, 22u);
		}
		{
			auto n = rsa::math::big_uint_16(23u);
			EXPECT_EQ(n--, 23u);
			EXPECT_EQ(n, 22u);
		}

		EXPECT_THROW(--rsa::math::big_uint_8(), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_8()--, std::invalid_argument);
	}

#pragma endregion

#pragma region comparison

	TEST(Test_RSA, math_equality__IsCorrect)
	{
		EXPECT_TRUE(rsa::math::big_uint_32(53u) == rsa::math::big_uint_32(53u));
		EXPECT_TRUE(rsa::math::big_uint_32(64u) == std::uint16_t{ 64u });
		EXPECT_TRUE(std::uint8_t{ 12u } == rsa::math::big_uint_8(12u));
	}

	TEST(Test_RSA, math_inequality__IsCorrect)
	{
		EXPECT_TRUE(rsa::math::big_uint_32(51u) != rsa::math::big_uint_32(53u));
		EXPECT_TRUE(rsa::math::big_uint_32(64334u) != std::uint16_t{ 2u });
		EXPECT_TRUE(std::uint8_t{ 12u } != rsa::math::big_uint_8(123u));
	}

	TEST(Test_RSA, math_lessthan__IsCorrect)
	{
		// same data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(53u) < rsa::math::big_uint_8(123u));
			EXPECT_FALSE(rsa::math::big_uint_32(utils::uint32_max) < rsa::math::big_uint_32(23u));
			EXPECT_FALSE(rsa::math::big_uint_16(utils::uint32_max) < rsa::math::big_uint_16(utils::uint32_max));
		}
		// different data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(12u) < rsa::math::big_uint_8(utils::uint16_max));
			EXPECT_FALSE(rsa::math::big_uint_16(utils::uint64_max) < rsa::math::big_uint_16(1234u));
		}
		{
			EXPECT_TRUE(3u < rsa::math::big_uint_32(12u));
		}
		{
			EXPECT_FALSE(rsa::math::big_uint_32(12u) < 12u);
		}
	}

	TEST(Test_RSA, math_greaterthan__IsCorrect)
	{
		// same data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(123u) > rsa::math::big_uint_8(53u));
			EXPECT_FALSE(rsa::math::big_uint_32(23u) > rsa::math::big_uint_32(utils::uint32_max));
			EXPECT_FALSE(rsa::math::big_uint_16(utils::uint32_max) > rsa::math::big_uint_16(utils::uint32_max));
		}
		// different data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(utils::uint16_max) > rsa::math::big_uint_8(12u));
			EXPECT_FALSE(rsa::math::big_uint_16(1234u) > rsa::math::big_uint_16(utils::uint64_max));
		}
		{
			EXPECT_TRUE(rsa::math::big_uint_32(12u) > 3u);
		}
		{
			EXPECT_FALSE(12u > rsa::math::big_uint_32(12u));
		}
	}

	TEST(Test_RSA, math_lessthanequal__IsCorrect)
	{
		// same data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(53u) <= rsa::math::big_uint_8(123u));
			EXPECT_FALSE(rsa::math::big_uint_32(utils::uint32_max) <= rsa::math::big_uint_32(23u));
			EXPECT_TRUE(rsa::math::big_uint_16(utils::uint32_max) <= rsa::math::big_uint_16(utils::uint32_max));
		}
		// different data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(12u) <= rsa::math::big_uint_8(utils::uint16_max));
			EXPECT_FALSE(rsa::math::big_uint_16(utils::uint64_max) <= rsa::math::big_uint_16(1234u));
		}
		{
			EXPECT_TRUE(3u <= rsa::math::big_uint_32(12u));
		}
		{
			EXPECT_TRUE(rsa::math::big_uint_32(12u) <= 12u);
		}
	}

	TEST(Test_RSA, math_greaterthanequal__IsCorrect)
	{
		// same data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(123u) >= rsa::math::big_uint_8(53u));
			EXPECT_FALSE(rsa::math::big_uint_32(23u) >= rsa::math::big_uint_32(utils::uint32_max));
			EXPECT_TRUE(rsa::math::big_uint_16(utils::uint32_max) >= rsa::math::big_uint_16(utils::uint32_max));
		}
		// different data sizes
		{
			EXPECT_TRUE(rsa::math::big_uint_8(utils::uint16_max) >= rsa::math::big_uint_8(12u));
			EXPECT_FALSE(rsa::math::big_uint_16(1234u) >= rsa::math::big_uint_16(utils::uint64_max));
		}
		{
			EXPECT_TRUE(rsa::math::big_uint_32(12u) >= 3u);
		}
		{
			EXPECT_TRUE(12u >= rsa::math::big_uint_32(12u));
		}
	}

#pragma endregion

	// TDOO (now):
		// make test names more consistent and remove redundant IsCorrect
		// test everything with bool (it's an unsigned type!)
		// test using lhs *= lhs, lhs += lhs, etc.
		// add more tests with 64 bit ints (will break accidental u32 stuff).
		// move math operations out of the class.

	// TODO (sometime):
		// support indexing of bits with big_uint instead of size_t?
		// construct from biguints with other block sizes
		// string constructor and to_string()

} // test
