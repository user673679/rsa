
#include <algorithm>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace rsa
{

	namespace utils
	{

		void die()
		{
			__debugbreak();
		}

		void die_if(bool condition)
		{
			if (condition)
				die();
		}

	} // utils

	namespace math
	{

		namespace meta
		{

			template<class uint_t>
			constexpr bool is_uint_v = (std::is_integral_v<uint_t> && std::is_unsigned_v<uint_t>);

			template<class uint_t>
			using enable_if_uint_t = std::enable_if_t<is_uint_v<uint_t>>;

		} // meta

		template<class block_t>
		class big_uint
		{
		public:

			using block_type = block_t;
			using data_type = std::vector<block_type>;

			static constexpr auto block_digits = std::numeric_limits<block_type>::digits;
			static constexpr auto block_max = std::numeric_limits<block_type>::max();

#pragma region constructors

			big_uint();

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			explicit big_uint(uint_t n);

			big_uint(big_uint const&) = default;
			big_uint(big_uint&&) = default;

#pragma endregion

#pragma region assignment

			big_uint& operator=(big_uint const&) = default;
			big_uint& operator=(big_uint&&) = default;

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator=(uint_t n);

#pragma endregion

#pragma region general

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			uint_t to_uint() const;

			data_type& data();
			data_type const& data() const;

#pragma endregion

#pragma region math operators

			big_uint& operator+=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator+=(uint_t n);

#pragma endregion

		private:

			data_type m_data;
		};

		using big_uint_8 = big_uint<std::uint8_t>;
		using big_uint_16 = big_uint<std::uint16_t>;
		using big_uint_32 = big_uint<std::uint32_t>;
		using big_uint_64 = big_uint<std::uint64_t>;

#pragma region construct

		template<class block_t>
		big_uint<block_t>::big_uint():
			big_uint(0u)
		{

		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>::big_uint(uint_t n)
		{
			// shifting by >= the number digits in the type is undefined behaviour.
			constexpr auto can_rshift = (block_digits < std::numeric_limits<uint_t>::digits);

			while (n != uint_t{ 0 })
			{
				// integer promotion, conversion to greater rank, implicit conversion to block_type
				m_data.push_back(block_max & n);

				if (can_rshift)
					n >>= block_digits;
				else
					n = uint_t{ 0 };
			}
		}

#pragma endregion

#pragma region general

		template<class block_t>
		template<class uint_t, typename>
		uint_t big_uint<block_t>::to_uint() const
		{
			// it's much easier to static_assert / throw here if uint_t may be too small.
			// checking the actual value would be a lot more work.
			{
				constexpr auto uint_digits = std::numeric_limits<uint_t>::digits;

				static_assert(block_digits <= uint_digits, "uint_t may be too small to represent this number.");

				if (m_data.size() * block_digits > uint_digits)
					throw std::range_error("uint_t may be too small to represent this number.");
			}

			auto result = uint_t{ 0 };

			if (m_data.empty())
				return result;

			for (auto i = std::size_t{ 0 }; i != m_data.size(); ++i)
				result |= (uint_t{ m_data[i] } << (i * block_digits));

			return result;
		}

		template<class block_t>
		typename big_uint<block_t>::data_type& big_uint<block_t>::data()
		{
			return m_data;
		}

		template<class block_t>
		typename big_uint<block_t>::data_type const& big_uint<block_t>::data() const
		{
			return m_data;
		}

#pragma endregion

#pragma region assignment

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator=(uint_t n)
		{
			return (*this = big_uint(n));
		}

#pragma endregion

#pragma region math operators

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator+=(big_uint const& b)
		{
			const auto get_block = [] (data_type const& data, std::size_t i)
			{
				return (i < data.size()) ? data[i] : block_type{ 0 };
			};

			const auto get_block_extend = [] (data_type& data, std::size_t i) -> block_type&
			{
				if (i == data.size())
					data.push_back(block_type{ 0 });

				return data[i];
			};

			const auto checked_addassign = [] (block_type& a, block_type b)
			{
				return ((a += b) < b);
			};

			auto& a = *this;
			auto carry = false;
			const auto max_size = std::max(a.data().size(), b.data().size());

			// add corresponding blocks. in case of overflow, carry one to the next block.
			for (auto i = std::size_t{ 0 }; i != max_size; ++i)
			{
				auto const& b_block = get_block(b.data(), i);
				auto& a_block = get_block_extend(a.data(), i);

				// use bitwise or so both sides are evaluated.
				carry = (checked_addassign(a_block, b_block) | checked_addassign(a_block, carry ? block_type{ 1 } : block_type{ 0 }));
			}

			if (carry)
				a.data().push_back(block_type{ 1 });

			return a;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator+=(uint_t n)
		{
			return operator+=(big_uint(n));
		}

#pragma endregion

#pragma region comparison

		template<class block_t>
		bool operator==(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			if (a.data().size() != b.data().size())
				return false;

			return std::equal(a.data().begin(), a.data().end(), b.data().begin());
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator==(big_uint<block_t> const& a, uint_t b)
		{
			return (a == big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator==(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) == b);
		}

		template<class block_t>
		bool operator!=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(a == b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator!=(big_uint<block_t> const& a, uint_t b)
		{
			return (a != big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator!=(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) != b);
		}

#pragma endregion

#pragma region math operators

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator+(big_uint<block_t> a, uint_t b)
		{
			return (a += big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator+(uint_t a, big_uint<block_t> b)
		{
			return (b += big_uint<block_t>(a));
		}

		template<class block_t>
		big_uint<block_t> operator+(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a += b);
		}

#pragma endregion



		bool is_prime(std::uint64_t n)
		{
			if (n % 2 == 0)
				return n == 2;

			if (n % 3 == 0)
				return n == 3;

			for (auto k = std::uint64_t{ 6 }; (k - 1) * (k - 1) <= n; k += 6)
			{
				if (n % (k - 1) == 0 || n % (k + 1) == 0)
					return false;
			}

			return true;
		}

	} // math


	//constexpr auto MAX_RANDOM_BITS = std::size_t{ 32768 };

	//std::vector<bool> generate_random_bits(std::mt19937_64& rng, std::size_t num_bits)
	//{
	//	if (num_bits > MAX_RANDOM_BITS)
	//	{
	//		auto const max_str = std::to_string(MAX_RANDOM_BITS);
	//		throw std::invalid_argument("`num_bits` must not be greater than " + max_str + ".");
	//	}

	//	auto const dist = std::uniform_int_distribution<int>(0, 1);

	//	auto result = std::vector<bool>(num_bits, false);
	//	utils::die_if(result.size() != num_bits);

	//	std::generate(result.begin(), result.end(), [&] () { return (dist(rng) == 1); });

	//	return result;
	//}

	//constexpr auto MIN_PRIME_BITS = std::size_t{ 16 };
	//constexpr auto MAX_PRIME_BITS = MAX_RANDOM_BITS;

	//std::vector<bool> generate_prime(std::size_t num_bits)
	//{
	//	if (num_bits < MIN_PRIME_BITS || num_bits > MAX_PRIME_BITS)
	//	{
	//		auto const min_str = std::to_string(MIN_PRIME_BITS);
	//		auto const max_str = std::to_string(MAX_PRIME_BITS);
	//		throw std::invalid_argument("`num_bits` must be greater than " + min_str + " and less than " + max_str + ".");
	//	}

	//	auto const seed = std::random_device()();
	//	auto rng = std::mt19937_64(seed);

	//	while (true)
	//	{
	//		auto random_bits = generate_random_bits(rng, num_bits);
	//		utils::die_if(random_bits.size() != num_bits);

	//		// set first (ensures odd number) and last (ensures large prime) num_bits
	//		random_bits[0] = true;
	//		random_bits[random_bits.size() - 1] = true;

	//		auto const two = math::to_bits(2);

	//		// search for prime number over an interval of 2^num_bits
	//		auto const search_distance = num_bits * std::size_t{ 2 };

	//		for (auto i = std::size_t{ 0 }; i != search_distance; ++i)
	//		{
	//			if (math::is_prime(random_bits))
	//				return random_bits;

	//			math::bits_add(random_bits, two);
	//		}
	//	}

	//	utils::die();
	//	return{ };
	//}

	math::big_uint_32 generate_random_bits(std::mt19937_64& , std::size_t )
	{
		auto result = math::big_uint_32();

		// ...


		return result;
	}

} // rsa


#include <gtest/gtest.h>

#include <numeric>
#include <algorithm>

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

#pragma region new

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
			auto n = rsa::math::big_uint_32(std::uint64_t{ std::numeric_limits<std::uint32_t>::max() } + 1u);
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
		EXPECT_EQ((rsa::math::big_uint_32(max) + 1u).to_uint<std::uint64_t>(), std::uint64_t{ max } + 1u);
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
			EXPECT_EQ(a, std::uint32_t{ max16 } + max16);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } + max16);
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
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } + 1u);
		}
		// a (one block), b (two blocks), ends in carry
		{
			auto max16 = std::numeric_limits<std::uint16_t>::max();
			auto a = rsa::math::big_uint_8(1u);
			auto b = rsa::math::big_uint_8(max16);
			a += b;
			EXPECT_EQ(a, std::uint32_t{ max16 } + 1u);
			EXPECT_EQ(a.to_uint<std::uint32_t>(), std::uint32_t{ max16 } + 1u);
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
			EXPECT_EQ(n, std::uint32_t{ max8 } + 2u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } + 2u);
		}
		{
			auto max8 = std::numeric_limits<std::uint8_t>::max();
			auto n = max8 + rsa::math::big_uint_8(1u);
			EXPECT_EQ(n, std::uint32_t{ max8 } + 1u);
			EXPECT_EQ(n.to_uint<std::uint32_t>(), std::uint32_t{ max8 } + 1u);
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

#pragma endregion

	// TDOO (now):
	// string constructor
	// to_string()
	// -= *= /=


	// TODO (sometime):
	// bitwise operators
	// construct from big_uints with other block sizes

} // test

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "...";

	return RUN_ALL_TESTS();
}
