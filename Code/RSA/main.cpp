
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

			// ... string constructor

			big_uint(big_uint const&) = default;
			big_uint(big_uint&&) = default;

#pragma endregion

#pragma region assignment

			big_uint& operator=(big_uint const&) = default;
			big_uint& operator=(big_uint&&) = default;

			// ... assign from value

#pragma endregion

#pragma region general

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			uint_t to_uint() const;

			// ... to_string()

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

#pragma region math - old

		std::vector<bool> to_bits(std::uint64_t n)
		{
			constexpr auto num_bits = std::numeric_limits<std::uint64_t>::digits;

			auto result = std::vector<bool>(num_bits, false);
			utils::die_if(result.size() != num_bits);

			for (auto i = std::size_t{ 0 }; i != num_bits; ++i)
				result[i] = ((n >> std::uint64_t{ i }) & std::uint64_t{ 1 });

			result.erase(std::find(result.rbegin(), result.rend(), true).base(), result.end());

			return result;
		}

		std::uint64_t to_number(std::vector<bool> const& bits)
		{
			constexpr auto num_bits = std::numeric_limits<std::uint64_t>::digits;

			if (bits.size() > num_bits)
			{
				auto digits_str = std::to_string(num_bits);
				throw std::invalid_argument("size of 'bits' must be less than or equal to " + digits_str + ".");
			}

			auto result = std::uint64_t{ 0 };

			for (auto i = bits.rbegin(); i != bits.rend(); ++i)
				result = (result << 1) | std::uint64_t{ *i };

			return result;
		}

		bool bits_at(std::vector<bool> const& a, std::size_t index)
		{
			if (index < a.size())
				return a[index];

			return false;
		}

		bool bits_equal(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			for (auto i = std::size_t{ 0 }; i != std::max(a.size(), b.size()); ++i)
			{
				if (!(bits_at(a, i) == bits_at(b, i)))
					return false;
			}

			return true;
		}

		bool bits_greater(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			auto size = std::max(a.size(), b.size());

			for (auto i = std::size_t{ 0 }; i != size; ++i)
			{
				auto const a_bit = bits_at(a, (size - 1) - i);
				auto const b_bit = bits_at(b, (size - 1) - i);

				if (a_bit && !b_bit)
					return true;
				else if (!a_bit && b_bit)
					return false;
			}

			return false;
		}

		std::vector<bool> bits_not_and(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			auto result = std::vector<bool>(std::max(a.size(), b.size()), false);

			for (auto i = std::size_t{ 0 }; i != result.size(); ++i)
				result[i] = !bits_at(a, i) && bits_at(b, i);

			return result;
		}

		std::vector<bool> bits_and(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			auto result = std::vector<bool>(std::max(a.size(), b.size()), false);

			for (auto i = std::size_t{ 0 }; i != result.size(); ++i)
				result[i] = (bits_at(a, i) && bits_at(b, i));

			return result;
		}

		std::vector<bool> bits_or(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			auto result = std::vector<bool>(std::max(a.size(), b.size()), false);

			for (auto i = std::size_t{ 0 }; i != result.size(); ++i)
				result[i] = (bits_at(a, i) || bits_at(b, i));

			return result;
		}

		std::vector<bool> bits_xor(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			auto result = std::vector<bool>(std::max(a.size(), b.size()), false);

			for (auto i = std::size_t{ 0 }; i != result.size(); ++i)
				result[i] = (bits_at(a, i) != bits_at(b, i));

			return result;
		}

		std::vector<bool> bits_lshift(std::vector<bool> a, std::size_t n)
		{
			a.insert(a.begin(), n, false);
			return a;
		}

		std::vector<bool> bits_rshift(std::vector<bool> a, std::size_t n)
		{
			a.erase(a.begin(), a.begin() + std::min(a.size(), n));
			return a;
		}

		std::vector<bool> bits_add(std::vector<bool> a, std::vector<bool> b)
		{
			auto const zero = std::vector<bool>{ };

			while (!bits_equal(b, zero))
			{
				auto carry = bits_and(a, b);
				a = bits_xor(a, b);
				b = bits_lshift(carry, 1);
			}

			return a;
		}

		std::vector<bool> bits_sub(std::vector<bool> a, std::vector<bool> b)
		{
			auto const zero = std::vector<bool>{ };

			if (bits_equal(b, zero))
				return a;

			if (bits_greater(b, a))
				throw std::invalid_argument("can't subtract larger number from smaller one!");

			while (!bits_equal(b, zero))
			{
				auto borrow = bits_not_and(a, b);
				a = bits_xor(a, b);
				b = bits_lshift(borrow, 1); // is this ok?
			}

			return a;
		}

		std::vector<bool> bits_mul(std::vector<bool> a, std::vector<bool> b)
		{
			auto const zero = std::vector<bool>{ };

			auto c = std::vector<bool>{ };

			while (!bits_equal(b, zero))
			{
				if (bits_at(b, 0))
					c = bits_add(c, a);

				a = bits_lshift(a, 1);
				b = bits_rshift(b, 1);
			}

			return c;
		}

		std::vector<bool> bits_div(std::vector<bool> n, std::vector<bool> d)
		{
			auto const zero = std::vector<bool>{ };

			if (bits_equal(d, zero))
				throw std::invalid_argument("divisor cannot be zero!");

			auto q = std::vector<bool>{ };

			while (!bits_greater(d, n))
			{
				auto i = std::size_t{ 0 };
				auto dt = d;

				while (!bits_greater(bits_lshift(dt, 1), n) && ++i)
					dt = bits_lshift(dt, 1);

				q = bits_or(q, bits_lshift(std::vector<bool>{ true }, i));
				n = bits_sub(n, dt);
			}

			return q;
		}

		std::vector<bool> bits_mod(std::vector<bool> const& a, std::vector<bool> const& b)
		{
			return bits_sub(a, bits_mul(bits_div(a, b), b));
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

		bool is_prime(std::vector<bool> const& n)
		{
			auto const zero = to_bits(0);
			auto const two = to_bits(2);

			if (bits_equal(bits_mod(n, two), zero))
				return bits_equal(n, two);

			auto const three = to_bits(3);

			if (bits_equal(bits_mod(n, three), zero))
				return bits_equal(n, three);

			auto const one = to_bits(1);
			auto const six = to_bits(6);

			for (auto k = six; bits_greater(n, bits_mul(bits_sub(k, one), bits_sub(k, one))); k = bits_add(k, six))
			{
				if (bits_equal(bits_mod(n, bits_sub(k, one)), zero) || bits_equal(bits_mod(n, bits_add(k, one)), zero))
					return false;
			}

			return true;
		}

	} // math

	constexpr auto MAX_RANDOM_BITS = std::size_t{ 32768 };

	std::vector<bool> generate_random_bits(std::mt19937_64& rng, std::size_t num_bits)
	{
		if (num_bits > MAX_RANDOM_BITS)
		{
			auto const max_str = std::to_string(MAX_RANDOM_BITS);
			throw std::invalid_argument("`num_bits` must not be greater than " + max_str + ".");
		}

		auto const dist = std::uniform_int_distribution<int>(0, 1);

		auto result = std::vector<bool>(num_bits, false);
		utils::die_if(result.size() != num_bits);

		std::generate(result.begin(), result.end(), [&] () { return (dist(rng) == 1); });

		return result;
	}

	constexpr auto MIN_PRIME_BITS = std::size_t{ 16 };
	constexpr auto MAX_PRIME_BITS = MAX_RANDOM_BITS;

	std::vector<bool> generate_prime(std::size_t num_bits)
	{
		if (num_bits < MIN_PRIME_BITS || num_bits > MAX_PRIME_BITS)
		{
			auto const min_str = std::to_string(MIN_PRIME_BITS);
			auto const max_str = std::to_string(MAX_PRIME_BITS);
			throw std::invalid_argument("`num_bits` must be greater than " + min_str + " and less than " + max_str + ".");
		}

		auto const seed = std::random_device()();
		auto rng = std::mt19937_64(seed);

		while (true)
		{
			auto random_bits = generate_random_bits(rng, num_bits);
			utils::die_if(random_bits.size() != num_bits);

			// set first (ensures odd number) and last (ensures large prime) num_bits
			random_bits[0] = true;
			random_bits[random_bits.size() - 1] = true;

			auto const two = math::to_bits(2);

			// search for prime number over an interval of 2^num_bits
			auto const search_distance = num_bits * std::size_t{ 2 };

			for (auto i = std::size_t{ 0 }; i != search_distance; ++i)
			{
				if (math::is_prime(random_bits))
					return random_bits;

				math::bits_add(random_bits, two);
			}
		}

		utils::die();
		return{ };
	}

	math::big_uint_32 bn_generate_random_bits(std::mt19937_64& , std::size_t )
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

	//TEST(Test_RSA, math_to_bits__GivesCorrectNumberOfBits)
	//{
	//	EXPECT_EQ(rsa::math::to_bits(0).size(), 0u);
	//	EXPECT_EQ(rsa::math::to_bits(1).size(), 1u);
	//	EXPECT_EQ(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max()).size(), std::numeric_limits<std::uint64_t>::digits);
	//	EXPECT_EQ(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max() - 1u).size(), std::numeric_limits<std::uint64_t>::digits);
	//}

	//TEST(Test_RSA, math_to_bits__GivesCorrectBitPattern)
	//{
	//	{
	//		auto one = std::vector<bool>(1, true);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(1), one));
	//	}
	//	{
	//		auto max = std::vector<bool>(std::numeric_limits<std::uint64_t>::digits, true);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max()), max));
	//	}
	//	{
	//		auto max_sub_one = std::vector<bool>(std::numeric_limits<std::uint64_t>::digits, true);
	//		max_sub_one[0] = false;
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max() - 1), max_sub_one));
	//	}
	//}

	//TEST(Test_RSA, math_to_number__ThrowsWithIncorrectNumberOfBits)
	//{
	//	EXPECT_THROW(rsa::math::to_number(std::vector<bool>(65, false)), std::invalid_argument);
	//}

	//TEST(Test_RSA, math_to_number__GivesCorrectValue)
	//{
	//	auto digits = std::numeric_limits<std::uint64_t>::digits;
	//	{
	//		auto one = std::vector<bool>(digits, false);
	//		one[0] = true;
	//		EXPECT_EQ(rsa::math::to_number(one), 1);
	//	}
	//	{
	//		auto max = std::vector<bool>(digits, true);
	//		EXPECT_EQ(rsa::math::to_number(max), std::numeric_limits<std::uint64_t>::max());
	//	}
	//	{
	//		auto max_sub_one = std::vector<bool>(digits, true);
	//		max_sub_one[0] = false;
	//		EXPECT_EQ(rsa::math::to_number(max_sub_one), std::numeric_limits<std::uint64_t>::max() - 1);
	//	}
	//}

	//TEST(Test_RSA, math_bits_at__ReturnsBitAtValidIndex)
	//{
	//	auto bits = std::vector<bool>(5, false);
	//	bits[0] = true;
	//	bits[4] = true;

	//	for (auto i = std::size_t{ 0 }; i != bits.size(); ++i)
	//		EXPECT_EQ(rsa::math::bits_at(bits, i), bits[i]);
	//}

	//TEST(Test_RSA, math_bits_at__ReturnsFalseAtInvalidIndex)
	//{
	//	auto bits = std::vector<bool>(5, true);
	//	EXPECT_EQ(rsa::math::bits_at(bits, bits.size()), false);
	//	EXPECT_EQ(rsa::math::bits_at(bits, std::numeric_limits<std::size_t>::max()), false);
	//}

	//TEST(Test_RSA, math_bits_equal__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>(5, false);
	//		auto b = std::vector<bool>();
	//		auto c = std::vector<bool>(1, false);
	//		EXPECT_TRUE(rsa::math::bits_equal(a, b));
	//		EXPECT_TRUE(rsa::math::bits_equal(b, a));
	//		EXPECT_TRUE(rsa::math::bits_equal(a, c));
	//		EXPECT_TRUE(rsa::math::bits_equal(c, a));
	//		EXPECT_TRUE(rsa::math::bits_equal(c, b));
	//		EXPECT_TRUE(rsa::math::bits_equal(b, c));
	//	}
	//	{
	//		auto a = std::vector<bool>(5, true);
	//		auto b = std::vector<bool>();
	//		auto c = std::vector<bool>(1, true);
	//		EXPECT_FALSE(rsa::math::bits_equal(a, b));
	//		EXPECT_FALSE(rsa::math::bits_equal(b, a));
	//		EXPECT_FALSE(rsa::math::bits_equal(a, c));
	//		EXPECT_FALSE(rsa::math::bits_equal(c, a));
	//		EXPECT_FALSE(rsa::math::bits_equal(c, b));
	//		EXPECT_FALSE(rsa::math::bits_equal(b, c));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false, true, false, false };
	//		auto b = std::vector<bool>{ true, false, true };
	//		EXPECT_TRUE(rsa::math::bits_equal(a, b));
	//		EXPECT_TRUE(rsa::math::bits_equal(b, a));
	//	}
	//}

	//TEST(Test_RSA, math_bits_greater)
	//{
	//	{
	//		auto a = std::vector<bool>(5, false);
	//		auto b = std::vector<bool>();
	//		auto c = std::vector<bool>(1, false);
	//		EXPECT_FALSE(rsa::math::bits_greater(a, b));
	//		EXPECT_FALSE(rsa::math::bits_greater(b, a));
	//		EXPECT_FALSE(rsa::math::bits_greater(a, c));
	//		EXPECT_FALSE(rsa::math::bits_greater(c, a));
	//		EXPECT_FALSE(rsa::math::bits_greater(c, b));
	//		EXPECT_FALSE(rsa::math::bits_greater(b, c));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true };
	//		EXPECT_FALSE(rsa::math::bits_greater(a, a));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_greater(a, b));
	//		EXPECT_FALSE(rsa::math::bits_greater(b, a));
	//	}
	//}

	//TEST(Test_RSA, math_bits_not_and__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ };
	//		auto b = std::vector<bool>{ };
	//		EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ });
	//	}
	//	{
	//		auto a = std::vector<bool>{ false };
	//		auto b = std::vector<bool>{ false };
	//		EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
	//	}
	//	{
	//		auto a = std::vector<bool>{ true };
	//		auto b = std::vector<bool>{ false };
	//		EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
	//	}
	//	{
	//		auto a = std::vector<bool>{ true };
	//		auto b = std::vector<bool>{ true };
	//		EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
	//	}
	//	{
	//		auto a = std::vector<bool>{ false };
	//		auto b = std::vector<bool>{ true };
	//		EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ true });
	//	}
	//}

	//TEST(Test_RSA, math_bits_and__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ false, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{}));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true, true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true, true }));
	//	}
	//}

	//TEST(Test_RSA, math_bits_or__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ false, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{}));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true, true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true, true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true, true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true, true }));
	//	}
	//}

	//TEST(Test_RSA, math_bits_xor__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ false, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{}));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ false, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, false };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, false };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ false, true }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ false, true }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ }));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ }));
	//	}
	//}

	//TEST(Test_RSA, math_bits_lshift__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ };
	//		EXPECT_EQ(rsa::math::bits_lshift(a, 0), a);
	//	}
	//	{
	//		auto a = std::vector<bool>{ };
	//		EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ false };
	//		EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false, false }));
	//	}
	//	{
	//		auto a = std::vector<bool>{ true };
	//		EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false, true }));
	//	}
	//}

	//TEST(Test_RSA, math_bits_rshift__IsCorrect)
	//{
	//	{
	//		auto a = std::vector<bool>{ };
	//		EXPECT_EQ(rsa::math::bits_rshift(a, 0), a);
	//	}
	//	{
	//		auto a = std::vector<bool>{ };
	//		EXPECT_EQ(rsa::math::bits_rshift(a, 1), a);
	//	}
	//	{
	//		auto a = std::vector<bool>(4, true);
	//		EXPECT_EQ(rsa::math::bits_rshift(a, 5), std::vector<bool>{ });
	//	}
	//	{
	//		auto a = std::vector<bool>{ true };
	//		EXPECT_EQ(rsa::math::bits_rshift(a, 1), std::vector<bool>{ });
	//	}
	//	{
	//		auto a = std::vector<bool>(4, true);
	//		EXPECT_EQ(rsa::math::bits_rshift(a, 2), std::vector<bool>(2, true));
	//	}
	//}

	//TEST(Test_RSA, math_bits_add__IsCorrect)
	//{
	//	{
	//		auto a = rsa::math::to_bits(0);
	//		auto b = rsa::math::to_bits(0);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(0)));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(0)));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(1);
	//		auto b = rsa::math::to_bits(0);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(1)));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(1)));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(1);
	//		auto b = rsa::math::to_bits(1);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(2)));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(2)));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(273);
	//		auto b = rsa::math::to_bits(54);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(273 + 54)));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(273 + 54)));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(2734756);
	//		auto b = rsa::math::to_bits(939876523);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(2734756 + 939876523)));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(2734756 + 939876523)));
	//	}
	//}

	//TEST(Test_RSA, math_bits_sub__WorksWithValidParameters)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(zero, zero), zero));
	//	}
	//	{
	//		auto one = std::vector<bool>{ true };
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(zero, zero), zero));
	//	}
	//	{
	//		auto big = rsa::math::to_bits(53);
	//		auto smol = rsa::math::to_bits(12);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(big, smol), rsa::math::to_bits(53 - 12)));
	//	}
	//	{
	//		auto big = rsa::math::to_bits(532387);
	//		auto smol = rsa::math::to_bits(5323);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(big, smol), rsa::math::to_bits(532387 - 5323)));
	//	}
	//	{
	//		auto max = rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max());
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(max, max), std::vector<bool>{ }));
	//	}
	//}

	//TEST(Test_RSA, math_bits_sub__ThrowsWithInvalidParameters)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_THROW(rsa::math::bits_sub(zero, one), std::invalid_argument);
	//	}
	//	{
	//		auto a = std::vector<bool>{ true, true };
	//		auto b = std::vector<bool>{ true, false, true };
	//		EXPECT_THROW(rsa::math::bits_sub(a, b), std::invalid_argument);
	//	}
	//}

	//TEST(Test_RSA, math_bits_mul__IsCorrect)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(zero, zero), zero));
	//	}
	//	{
	//		auto zero = std::vector<bool>{ };
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(zero, one), zero));
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(one, zero), zero));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(53u);
	//		auto r = rsa::math::to_bits(53u * 53u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(a, a), r));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(923u);
	//		auto b = rsa::math::to_bits(1u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(a, b), a));
	//	}
	//}

	//TEST(Test_RSA, math_bits_div__DivisionByZeroThrows)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_THROW(rsa::math::bits_div(zero, zero), std::invalid_argument);
	//	}
	//	{
	//		auto one = std::vector<bool>{ true };
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_THROW(rsa::math::bits_div(one, zero), std::invalid_argument);
	//	}
	//}

	//TEST(Test_RSA, math_bits_div__WorksWithValidParameters)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(zero, one), zero));
	//	}
	//	{
	//		auto zero = std::vector<bool>{ };
	//		auto num = rsa::math::to_bits(1234u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(zero, num), zero));
	//	}
	//	{
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(one, one), one));
	//	}
	//	{
	//		auto num = rsa::math::to_bits(1234u);
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(num, num), one));
	//	}
	//	{
	//		auto max = rsa::math::to_bits(4u);
	//		auto two = rsa::math::to_bits(2u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), two));
	//	}
	//	{
	//		auto max = rsa::math::to_bits(5u);
	//		auto two = rsa::math::to_bits(2u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), two));
	//	}
	//	{
	//		auto max = rsa::math::to_bits(3u);
	//		auto two = rsa::math::to_bits(2u);
	//		auto one = rsa::math::to_bits(1u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), one));
	//	}
	//}

	//TEST(Test_RSA, math_bits_mod__ModulusZeroThrows)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_THROW(rsa::math::bits_mod(zero, zero), std::invalid_argument);
	//	}
	//	{
	//		auto one = std::vector<bool>{ true };
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_THROW(rsa::math::bits_mod(one, zero), std::invalid_argument);
	//	}
	//}

	//TEST(Test_RSA, math_bits_mod__IsCorrect)
	//{
	//	{
	//		auto zero = std::vector<bool>{ };
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(zero, one), zero));
	//	}

	//	{
	//		auto one = std::vector<bool>{ true };
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(one, one), zero));
	//	}
	//	{
	//		auto two = std::vector<bool>{ false, true };
	//		auto one = std::vector<bool>{ true };
	//		auto zero = std::vector<bool>{ };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(two, one), zero));
	//	}
	//	{
	//		auto three = std::vector<bool>{ true, true };
	//		auto two = std::vector<bool>{ false, true };
	//		auto one = std::vector<bool>{ true };
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(three, two), one));
	//	}
	//	{
	//		auto a = rsa::math::to_bits(342378u);
	//		auto b = rsa::math::to_bits(448u);
	//		auto c = rsa::math::to_bits(342378u % 448u);
	//		EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(a, b), c));
	//	}
	//}

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

	// TODO: value assignment

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

		// ... test more values!
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

	// TODO (sometime): operations on numbers with different block sizes, or a way to cast between them.

} // test

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "...";

	return RUN_ALL_TESTS();
}
