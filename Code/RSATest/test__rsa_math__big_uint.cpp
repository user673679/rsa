
#include "rsa_math__big_uint.h"
#include "test_utils__uint_max.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <random>

namespace test
{

#pragma region constructors

	TEST(Test_RSA, math_big_uint_default_constructor__IsZero)
	{
		{
			auto n = rsa::math::big_uint_32();
			EXPECT_TRUE(n.data().empty());
			EXPECT_TRUE(n.is_zero());
			EXPECT_EQ(n, 0u);
		}
	}

	TEST(Test_RSA, math_big_uint_numeric_constructor)
	{
		{
			auto a = rsa::math::big_uint_32(std::uint16_t{ 37 });
			EXPECT_EQ(a.data().size(), 1u);
			EXPECT_EQ(a.data()[0], 37u);

			auto b = rsa::math::big_uint_32(std::uint64_t{ 37 });
			EXPECT_EQ(b.data().size(), 1u);
			EXPECT_EQ(b.data()[0], 37u);

			EXPECT_EQ(a, b);
		}
	}

	TEST(Test_RSA, math_big_uint_numeric_constructor__BitPattern)
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

	TEST(Test_RSA, math_big_uint_value_assign)
	{
		auto max = utils::uint64_max;
		auto n = rsa::math::big_uint_32(5u);
		n = max;

		EXPECT_EQ(n, max);
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

	TEST(Test_RSA, math_big_uint_to_uint)
	{
		const auto max = utils::uint32_max;

		EXPECT_EQ(rsa::math::big_uint_8(max).to_uint<std::uint32_t>(), max);
		EXPECT_EQ((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint64_t>(), std::uint64_t{ max } +1u);
	}

	TEST(Test_RSA, math_big_uint_is_zero)
	{
		EXPECT_TRUE(rsa::math::big_uint_16().is_zero());
		EXPECT_TRUE(rsa::math::big_uint_16(0u).is_zero());
		EXPECT_TRUE((rsa::math::big_uint_16(53u) - 53u).is_zero());
		EXPECT_FALSE(rsa::math::big_uint_32(1u).is_zero());
		EXPECT_FALSE(rsa::math::big_uint_32(utils::uint64_max).is_zero());
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

	TEST(Test_RSA, math_big_uint_get_most_significant_bit)
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

	TEST(Test_RSA, math_big_uint_bit_and_assign)
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

	TEST(Test_RSA, math_big_uint_bit_and)
	{
		auto a = rsa::math::big_uint_8(123987u);
		EXPECT_EQ(a & 19787u, std::uint32_t{ 123987u } & 19787u);
	}

	TEST(Test_RSA, math_big_uint_bit_or_assign)
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
			EXPECT_EQ(a |= b, 12395787u | 19787u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			EXPECT_EQ(a |= a, 12395787u);
		}
	}

	TEST(Test_RSA, math_big_uint_bit_or)
	{
		auto a = rsa::math::big_uint_8(127u);
		EXPECT_EQ(a | 19787u, 127u | 19787u);
	}

	TEST(Test_RSA, math_big_uint_bit_xor_assign)
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
			EXPECT_EQ(a ^= b, 12395787u ^ 19787u);
		}
		{
			auto a = rsa::math::big_uint_8(12395787u);
			EXPECT_TRUE((a ^= a).is_zero());
		}
	}

	TEST(Test_RSA, math_big_uint_bit_xor)
	{
		auto a = rsa::math::big_uint_8(127u);
		EXPECT_EQ(a ^ 19787u, 127u ^ 19787u);
	}

	TEST(Test_RSA, math_big_uint_lshift_assign)
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
		// extra test
		{
			auto n = rsa::math::big_uint_8(12u);
			EXPECT_EQ(n <<= 1u, 12u << 1u);
		}
	}

	TEST(Test_RSA, math_big_uint_lshift)
	{
		auto n = rsa::math::big_uint_8(1u);
		EXPECT_EQ((n << 1u), (1u << 1u));
	}

	TEST(Test_RSA, math_big_uint_rshift_assign)
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

	TEST(Test_RSA, math_big_uint_rshift)
	{
		auto n = rsa::math::big_uint_8(1u);
		EXPECT_TRUE((n >> 1u).is_zero());
	}

#pragma endregion

#pragma region math operators

	TEST(Test_RSA, math_big_uint_addassign__BigUint)
	{
		// a (two blocks), b (two blocks), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } + max16);
		}
		// a (two blocks), b (two blocks), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max16 / 2u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 / 2u + max16 / 2u });
		}
		// a (two blocks), b (one block), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(max16);
			auto b = rsa::math::big_uint_8(1u);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } + 1u);
		}
		// a (one block), b (two blocks), ends in carry
		{
			auto max16 = utils::uint16_max;
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } + 1u);
		}
		// a (two blocks), b (one block), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_8(max16 / 2u);
			auto b = rsa::math::big_uint_8(max8);
			a += b;
			EXPECT_EQ(a, (max16 / 2u) + max8);
		}
		// a (one block), b (two blocks), doesn't end in carry
		{
			auto max16 = utils::uint16_max;
			auto max8 = utils::uint8_max;
			auto a = rsa::math::big_uint_8(max8);
			auto b = rsa::math::big_uint_8(max16 / 2u);
			a += b;
			EXPECT_EQ(a, (max16 / 2u) + max8);
		}
		// self assign
		{
			auto a = rsa::math::big_uint_8(2u);
			a += a;
			EXPECT_EQ(a, 4u);
		}
		{
			auto a = rsa::math::big_uint_8(utils::uint16_max);
			a += a;
			EXPECT_EQ(a, 2u * utils::uint16_max);
		}
	}

	TEST(Test_RSA, math_big_uint_addassign__Value)
	{
		// assigning with zero
		{
			auto n = rsa::math::big_uint_32(0u);
			n += rsa::math::big_uint_32(0u);
			EXPECT_EQ(n, 0u);
		}
		{
			auto max = utils::uint32_max;
			auto n = rsa::math::big_uint_32(0u);
			n += max;
			EXPECT_EQ(n, max);
		}
		{
			auto max = utils::uint32_max;
			auto n = rsa::math::big_uint_32(max);
			n += 0u;
			EXPECT_EQ(n, max);
		}
		// assigning inside block
		{
			auto max = utils::uint8_max;
			auto n = rsa::math::big_uint_16(max);
			n += max;
			EXPECT_EQ(n, std::uint32_t{ max } + max);
		}
		// assigning outside block
		{
			auto max = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max);
			n += 1u;
			EXPECT_EQ(n, std::uint16_t{ max } + 1u);
		}
		{
			auto max16 = utils::uint16_max;
			auto max32 = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max16);
			n += max32;
			EXPECT_EQ(n, std::uint64_t{ max32 } + max16);
		}
	}

	TEST(Test_RSA, math_big_uint_add)
	{
		{
			auto n = rsa::math::big_uint_32(23u) + rsa::math::big_uint_32(78u);
			EXPECT_EQ(n, 23u + 78u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), 23u + 78u);
		}
		{
			auto max8 = utils::uint8_max;
			auto n = rsa::math::big_uint_8(max8) + 2u;
			EXPECT_EQ(n, std::uint32_t{ max8 } + 2u);
		}
		{
			auto max8 = utils::uint8_max;
			auto n = max8 + rsa::math::big_uint_8(1u);
			EXPECT_EQ(n, std::uint32_t{ max8 } + 1u);
		}
	}

	TEST(Test_RSA, math_big_uint_subassign__InvalidArgumentsThrow)
	{
		EXPECT_THROW(rsa::math::big_uint_32(0u) -= rsa::math::big_uint_32(1u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_32(5u) -= rsa::math::big_uint_32(6u), std::invalid_argument);
	}

	TEST(Test_RSA, math_big_uint_subassign__BigUint)
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

	TEST(Test_RSA, math_big_uint_subassign__Value)
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

	TEST(Test_RSA, math_big_uint_sub)
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

	TEST(Test_RSA, math_big_uint_mulassign__BigUint)
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
		// self assign
		{
			auto a = rsa::math::big_uint_8(5u);
			a *= a;
			EXPECT_EQ(a, 25u);
		}
	}

	TEST(Test_RSA, math_big_uint_mulassign__Value)
	{
		auto a = rsa::math::big_uint_16(utils::uint8_max);
		EXPECT_EQ(a *= utils::uint8_max, std::uint32_t{ utils::uint8_max } *utils::uint8_max);
	}

	TEST(Test_RSA, math_big_uint_mul)
	{
		{
			auto a = rsa::math::big_uint_16(utils::uint8_max);
			auto b = rsa::math::big_uint_16(utils::uint16_max);
			EXPECT_EQ(a * b, std::uint32_t{ utils::uint16_max } * utils::uint8_max);
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

	TEST(Test_RSA, math_big_uint_divassign__DivisionByZeroThrows)
	{
		EXPECT_THROW(rsa::math::big_uint_8(0u) /= rsa::math::big_uint_8(0u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_8(1u) /= rsa::math::big_uint_8(0u), std::invalid_argument);
	}

	TEST(Test_RSA, math_big_uint_divassign__BigUint)
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
		{
			auto a = rsa::math::big_uint_8(utils::uint64_max);
			a /= a;
			EXPECT_EQ(a, 1u);
		}

		auto rng = std::mt19937_64();
		auto dist = std::uniform_int_distribution<std::uint64_t>(0u, std::numeric_limits<std::uint64_t>::max());

		for (auto i = 0u; i != 10000u; ++i)
		{
			auto an = dist(rng);
			auto a = rsa::math::big_uint_8(an);
			auto bn = dist(rng);
			auto b = rsa::math::big_uint_8(bn);

			EXPECT_EQ(a / b, an / bn);
		}
	}

	TEST(Test_RSA, math_big_uint_divassign__Value)
	{
		auto a = rsa::math::big_uint_16(utils::uint8_max);
		EXPECT_EQ(a /= 2u, utils::uint8_max / 2u);
	}

	TEST(Test_RSA, math_big_uint_div)
	{
		EXPECT_EQ(rsa::math::big_uint_32(64u) / rsa::math::big_uint_32(8u), 8u);
		EXPECT_EQ(rsa::math::big_uint_8(257u) / 255u, 1u);
		EXPECT_EQ(908734u / rsa::math::big_uint_16(utils::uint64_max), 0u);
	}

	TEST(Test_RSA, math_big_uint_modassign__ModulusByZeroThrows)
	{
		EXPECT_THROW(rsa::math::big_uint_8(0u) %= rsa::math::big_uint_8(0u), std::invalid_argument);
		EXPECT_THROW(rsa::math::big_uint_8(1u) %= rsa::math::big_uint_8(0u), std::invalid_argument);
	}

	TEST(Test_RSA, math_big_uint_modassign__BigUint)
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
		{
			auto a = rsa::math::big_uint_8(96u);
			a %= a;
			EXPECT_EQ(a, 0u);
		}

		auto rng = std::mt19937_64();
		auto dist = std::uniform_int_distribution<std::uint64_t>(0u, std::numeric_limits<std::uint64_t>::max());

		for (auto i = 0u; i != 10000u; ++i)
		{
			auto an = dist(rng);
			auto a = rsa::math::big_uint_8(an);
			auto bn = dist(rng);
			auto b = rsa::math::big_uint_8(bn);

			EXPECT_EQ(a % b, an % bn);
		}
	}

	TEST(Test_RSA, math_big_uint_modassign__Value)
	{
		auto a = rsa::math::big_uint_16(utils::uint8_max);
		EXPECT_EQ(a %= 2u, 1u);
	}

	TEST(Test_RSA, math_big_uint_mod)
	{
		EXPECT_EQ(rsa::math::big_uint_32(64u) % rsa::math::big_uint_32(8u), 0u);
		EXPECT_EQ(rsa::math::big_uint_8(257u) % 255u, 2u);
		EXPECT_EQ(908734u % rsa::math::big_uint_16(utils::uint64_max), 908734u);
	}

	TEST(Test_RSA, math_big_uint_increment)
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

	TEST(Test_RSA, math_big_uint_decrement)
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

	TEST(Test_RSA, math_big_uint_equality)
	{
		EXPECT_TRUE(rsa::math::big_uint_32(53u) == rsa::math::big_uint_32(53u));
		EXPECT_TRUE(rsa::math::big_uint_32(64u) == std::uint16_t{ 64u });
		EXPECT_TRUE(std::uint8_t{ 12u } == rsa::math::big_uint_8(12u));
	}

	TEST(Test_RSA, math_big_uint_inequality)
	{
		EXPECT_TRUE(rsa::math::big_uint_32(51u) != rsa::math::big_uint_32(53u));
		EXPECT_TRUE(rsa::math::big_uint_32(64334u) != std::uint16_t{ 2u });
		EXPECT_TRUE(std::uint8_t{ 12u } != rsa::math::big_uint_8(123u));
	}

	TEST(Test_RSA, math_big_uint_lessthan)
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

	TEST(Test_RSA, math_big_uint_greaterthan)
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

	TEST(Test_RSA, math_big_uint_lessthanequal)
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

	TEST(Test_RSA, math_big_uint_greaterthanequal)
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

	TEST(Test_RSA, math_big_uint_newdiv_broken)
	{
		{
			using v16 = std::vector<std::uint16_t>;

			auto bu16 = [] (v16 const& values)
			{
				auto result = rsa::math::big_uint_16();

				for (auto v = values.rbegin(); v != values.rend(); ++v)
					result = (result << 16u) | *v;

				rsa::math::ops::utils::trim(result);
				return result;
			};

			auto test_cases = std::vector<std::vector<v16>>
			{
				{ { 0x1dc4, 0xe64a, }, { 0xb64f, 1, }, { 0x8680, }, { 0x9c44, 1, }, },
				{ { 0xbd80, 0x517c, 0x7db3, 0xdb5, }, { 0xaea7, 0x8f1c, }, { 0xce79, 0x1885, }, { 0xce91, 0x33a0, }, },
				{ { 0x2a40, 0x979c, 0x8267, 0x5b2, }, { 0xaea7, 0x8f1c, }, { 0xfe60, 0xa30, }, { 0xf9a0, 0x3240, }, },
				{ { 0xa400, 0x4368, 0x7200, 0x9dd, }, { 0xaea7, 0x8f1c, }, { 0x8996, 0x11a5, }, { 0xef26, 0x667f, }, },
			};

			for (auto const& t : test_cases)
			{
				{
					auto q = rsa::math::big_uint<std::uint16_t>();
					auto r = rsa::math::big_uint<std::uint16_t>();
					EXPECT_EQ(0, rsa::math::ops::call_divmnu(q, r, bu16({ t[0] }), bu16({ t[1] })));
					EXPECT_EQ(t[2], q.data());
					EXPECT_EQ(t[3], r.data());
				}

				{
					auto q = rsa::math::big_uint<std::uint16_t>();
					auto r = rsa::math::big_uint<std::uint16_t>();
					rsa::math::ops::div_test<std::uint16_t, std::uint32_t>(q, r, bu16({ t[0] }), bu16({ t[1] }));
					EXPECT_EQ(t[2], q.data());
					EXPECT_EQ(t[3], r.data());
				}
			}
		}
		{
			using v32 = std::vector<std::uint32_t>;

			auto bu32 = [] (v32 const& values)
			{
				auto result = rsa::math::big_uint_32();

				for (auto v = values.rbegin(); v != values.rend(); ++v)
					result = (result << 32u) | *v;

				rsa::math::ops::utils::trim(result);
				return result;
			};

			auto test_cases = std::vector<std::vector<v32>>
			{
				{ { 0x9b438190, 0xb73eb4c5, 0x85acfde2, 0xc3f79291, }, { 0x8974b469, 0xa9c0e07d, 1, }, { 0x22cf1ee8, 0x75d51bf4, }, { 0x8974b468, 0xa9c0e07d, 1, }, },
				{ { 0xe7581f03, 0x1740184e, 0xf67110ab, 0xcc60b2af, }, { 0xe71ce053, 0xcf5d0b26, 1, }, { 0x3b71146b, 0x70ea3f67, }, { 0xe71ce052, 0xcf5d0b26, 1, }, },
				{ { 0x2d44c289, 0xc19fe3a2, 0xe88ab547, 0xec81988a, 2, }, { 0x8d8e65c9, 0xd59044b5, 1, }, { 0xbcd99b39, 0x9813721f, 1, }, { 0x8d8e65c8, 0xd59044b5, 1, }, },
				{ { 0x569286e0, 0xbb0a31a6, 0x5dd10bbd, 0x7180c1b6, 0x25d5f0b3, 0xf60aa1e, 0xb15fc7f6, 0x296f4f60, }, { 0xf6f6aee5, 0x8bc80f1c, 0xabe457f8, 0xcdb91392, }, { 0xe7757c4c, 0x18c0a9a, 0x2914989c, 0x338fa6de, }, { 0xf6f6aee4, 0x8bc80f1c, 0xabe457f8, 0xcdb91392, }, },
			};

			for (auto const& t : test_cases)
			{
				{
					auto q = rsa::math::big_uint<std::uint32_t>();
					auto r = rsa::math::big_uint<std::uint32_t>();
					EXPECT_EQ(0, rsa::math::ops::call_divmnu(q, r, bu32({ t[0] }), bu32({ t[1] })));
					EXPECT_EQ(t[2], q.data());
					EXPECT_EQ(t[3], r.data());
				}

				{
					auto q = rsa::math::big_uint<std::uint32_t>();
					auto r = rsa::math::big_uint<std::uint32_t>();
					rsa::math::ops::div_test<std::uint32_t, std::uint64_t>(q, r, bu32({ t[0] }), bu32({ t[1] }));
					EXPECT_EQ(t[2], q.data());
					EXPECT_EQ(t[3], r.data());
				}
			}
		}
	}

	//TEST(Test_RSA, math_big_uint__newdiv)
	//{
	//	//{
	//	//	auto a = rsa::math::big_uint_32(23487u);
	//	//	auto b = rsa::math::big_uint_32(9847u);
	//	//	EXPECT_EQ(a / b, 23487u / 9847u);
	//	//}
	//	//{
	//	//	auto an = std::uint64_t{ 52 } << rsa::math::meta::digits<std::uint32_t>();
	//	//	auto a = rsa::math::big_uint_32(an);
	//	//	auto b = rsa::math::big_uint_32(9847u);
	//	//	EXPECT_EQ(a / b, an / 9847u);
	//	//}
	//	//{
	//	//	auto a = ((((rsa::math::big_uint_32(53u) << 32u) | 84739u) << 32u) | 256u);
	//	//	auto b = rsa::math::big_uint_32(9847u);
	//	//	auto q = (rsa::math::big_uint_32(23117025u) << 32u) + 2726935669u;
	//	//	EXPECT_EQ(a / b, q);
	//	//}

	//	using v16 = std::vector<std::uint16_t>;

	//	auto const zero = rsa::math::big_uint_16();

	//	auto bu16 = [] (v16 const& values)
	//	{
	//		auto result = rsa::math::big_uint_16();

	//		for (auto v = values.rbegin(); v != values.rend(); ++v)
	//			result = (result << 16u) | *v;

	//		rsa::math::ops::utils::trim(result);

	//		return result;
	//	};

	//	auto run_test = [&] (v16 const& dividend, v16 const& divisor, v16 const& expect_quotient, v16 const& expect_remainder)
	//	{
	//		auto q = zero; auto r = zero;
	//		rsa::math::ops::div_test<std::uint16_t, std::uint32_t>(q, r, bu16(dividend), bu16(divisor));
	//		return (q == bu16(expect_quotient) && r == bu16(expect_remainder));
	//	};

	//	// copied from hacker's delight testing for divmnu
	//	auto test_cases = std::vector<std::vector<v16>>
	//	{
	//		{ { 7 }, { 1, 3 }, { 0 }, { 7, 0 } },
	//		{ { 0, 0 }, { 1, 0 }, { 0 }, { 0 } },
	//		{ { 3 }, { 2 }, { 1 }, { 1 } },
	//		{ { 3 }, { 3 }, { 1 }, { 0 } },
	//		{ { 3 }, { 4 }, { 0 }, { 3 } },
	//		{ { 0 }, { 0xffff }, { 0 }, { 0 } },
	//		{ { 0xffff }, { 1 }, { 0xffff }, { 0 } },
	//		{ { 0xffff }, { 0xffff }, { 1 }, { 0 } },
	//		{ { 0xffff }, { 3 }, { 0x5555 }, { 0 } },
	//		{ { 0xffff, 0xffff }, { 1 }, { 0xffff, 0xffff }, { 0 } },
	//		{ { 0xffff, 0xffff }, { 0xffff }, { 1, 1 }, { 0 } },
	//		{ { 0xffff, 0xfffe }, { 0xffff }, { 0xffff, 0 }, { 0xfffe } },
	//		{ { 0x5678, 0x1234 }, { 0x9abc }, { 0x1e1e, 0 }, { 0x2c70 } },
	//		{ { 0, 0 }, { 0, 1 }, { 0 }, { 0 } },
	//		{ { 0, 7 }, { 0, 3 }, { 2 }, { 0, 1 } },
	//		{ { 5, 7 }, { 0, 3 }, { 2 }, { 5, 1 } },
	//		{ { 0, 6 }, { 0, 2 }, { 3 }, { 0 } },
	//		{ { 0x0001, 0x8000 }, { 0x7000, 0x4000 }, { 1 }, { 0x9001, 0x3fff } },
	//		{ { 0x789a, 0xbcde }, { 0x789a, 0xbcde }, { 1 }, { 0 } },
	//		{ { 0x789b, 0xbcde }, { 0x789a, 0xbcde }, { 1 }, { 1, 0 } },
	//		{ { 0x7899, 0xbcde }, { 0x789a, 0xbcde }, { 0 }, { 0x7899, 0xbcde } },
	//		{ { 0xffff, 0xffff }, { 0xffff, 0xffff }, { 1 }, { 0 } },
	//		{ { 0xffff, 0xffff }, { 0x0000, 0x0001 }, { 0xffff }, { 0xffff, 0x0000 } },
	//		{ { 0x89ab, 0x4567, 0x0123 }, { 0x0000, 0x0001 }, { 0x4567, 0x0123 }, { 0x89ab, 0x0000 } },
	//		{ { 0x0000, 0xfffe, 0x8000 }, { 0xffff, 0x8000 }, { 0xffff, 0x0000 }, { 0xffff, 0x7fff } }, // qhat = b + 1
	//		{ { 0x0003, 0x0000, 0x8000 }, { 0x0001, 0x0000, 0x2000 }, { 0x0003 }, { 0, 0, 0x2000 } }, // requires add back step
	//		{ { 0, 0, 0x8000, 0x7fff }, { 1, 0, 0x8000 }, { 0xfffe, 0 }, { 2, 0xffff, 0x7fff } }, // requires add back step
	//		{ { 0, 0xfffe, 0, 0x8000 }, { 0xffff, 0, 0x8000 }, { 0xffff, 0 }, { 0xffff, 0xffff, 0x7fff } }, // mult-sub quantity can't be treated as signed (?)
	//	};

	//	for (auto const& t : test_cases)
	//		EXPECT_TRUE(run_test(t[0], t[1], t[2], t[3]));

	//	// division by zero
	//	{
	//		auto q = zero; auto r = zero;
	//		EXPECT_THROW((rsa::math::ops::div_test<std::uint16_t, std::uint32_t>(q, r, bu16({ 3u }), bu16({ 0u }))), std::invalid_argument);
	//	}

	//	// TODO: lhs(9b438190 b73eb4c5 85acfde2 c3f79291), rhs(8974b469 a9c0e07d 1) -> (8974b468 a9c0e07d 1)
	//}

} // test
