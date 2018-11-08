
#include "rsa_math__big_uint.h"

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
			auto n = rsa::math::big_uint_32(std::numeric_limits<std::uint32_t>::max());
			EXPECT_EQ(n.data().size(), 1);
			EXPECT_EQ(n.data()[0], std::numeric_limits<std::uint32_t>::max());
		}
		{
			auto n = rsa::math::big_uint_32(std::uint64_t{ std::numeric_limits<std::uint32_t>::max() } +1u);
			EXPECT_TRUE(n.data().size() == 2);
			EXPECT_EQ(n.data()[0], 0u);
			EXPECT_EQ(n.data()[1], 1u);
		}
		{
			auto n = rsa::math::big_uint_32(std::numeric_limits<std::uint64_t>::max());
			EXPECT_TRUE(n.data().size() == 2);
			EXPECT_EQ(n.data()[0], std::numeric_limits<std::uint32_t>::max());
			EXPECT_EQ(n.data()[1], std::numeric_limits<std::uint32_t>::max());
		}
		{
			auto n = rsa::math::big_uint_8(std::numeric_limits<std::uint64_t>::max());
			EXPECT_TRUE(n.data().size() == 8);
			for (auto i = std::size_t{ 0 }; i != 8; ++i)
				EXPECT_EQ(n.data()[i], std::numeric_limits<std::uint8_t>::max());
		}
	}

	// TODO: construct from big_uint with different block_type

#pragma endregion

#pragma region assignment

	// not testing copy / move, as they're simple defaults

	TEST(Test_RSA, math_big_uint_value_assign__Works)
	{
		auto max = std::numeric_limits<std::uint64_t>::max();
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
		const auto max = std::numeric_limits<std::uint32_t>::max();

		EXPECT_THROW(rsa::math::big_uint_8(max).to_uint<std::uint8_t>(), std::range_error);
		EXPECT_THROW((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint32_t>(), std::range_error);
	}

	TEST(Test_RSA, math_big_uint_to_uint__ReturnsCorrectValueWhenInRange)
	{
		const auto max = std::numeric_limits<std::uint32_t>::max();

		EXPECT_EQ(rsa::math::big_uint_8(max).to_uint<std::uint32_t>(), max);
		EXPECT_EQ((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint64_t>(), std::uint64_t{ max } +1u);
	}

#pragma endregion

#pragma region math operators

	TEST(Test_RSA, math_addassign__BigUIntIsCorrect)
	{
		// a (two blocks), b (two blocks), ends in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +max16);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +max16);
		}
		// a (two blocks), b (two blocks), doesn't end in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max16 / 2u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 / 2u + max16 / 2u });
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 / 2u + max16 / 2u });
		}
		// a (two blocks), b (one block), ends in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(1u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +1u);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +1u);
		}
		// a (one block), b (two blocks), ends in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } +1u);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } +1u);
		}
		// a (two blocks), b (one block), doesn't end in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto max8 = std::numeric_limits<std::uint8_t>::max();
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max8);
			a += b;
			EXPECT_EQ(a, (max16 / 2u) + max8);
			EXPECT_EQ(a.to_uint<std::uint16_t>(), (max16 / 2u) + max8);
		}
		// a (one block), b (two blocks), doesn't end in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto max8 = std::numeric_limits<std::uint8_t>::max();
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
			auto max = std::numeric_limits<std::uint32_t>::max();
			auto n = rsa::math::big_uint_32(0u);
			n += max;
			EXPECT_EQ(n, max);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), max);
		}
		{
			auto max = std::numeric_limits<std::uint32_t>::max();
			auto n = rsa::math::big_uint_32(max);
			n += 0u;
			EXPECT_EQ(n, max);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), max);
		}
		// assigning inside block
		{
			auto max = std::numeric_limits<std::uint8_t>::max();
			auto n = rsa::math::big_uint_16(max);
			n += max;
			EXPECT_EQ(n, std::uint32_t{ max } +max);
			EXPECT_EQ(n.to_uint<std::uint16_t>(), std::uint16_t{ max } +max);
		}
		// assigning outside block
		{
			auto max = std::numeric_limits<std::uint8_t>::max();
			auto n = rsa::math::big_uint_8(max);
			n += 1u;
			EXPECT_EQ(n, std::uint16_t{ max } +1u);
			EXPECT_EQ(n.to_uint<std::uint16_t>(), std::uint16_t{ max } +1u);
		}
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto max32 = std::numeric_limits<std::uint8_t>::max();
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
			auto max8 = std::numeric_limits<std::uint8_t>::max();
			auto n = rsa::math::big_uint_8(max8) + 2u;
			EXPECT_EQ(n, std::uint32_t{ max8 } +2u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } +2u);
		}
		{
			auto max8 = std::numeric_limits<std::uint8_t>::max();
			auto n = max8 + rsa::math::big_uint_8(1u);
			EXPECT_EQ(n, std::uint32_t{ max8 } +1u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } +1u);
		}
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

#pragma endregion

	// TDOO (now):
		// string constructor
		// to_string()
		// -= *= /=

	// TODO (sometime):
		// bitwise operators
		// construct from big_uints with other block sizes

} // test
