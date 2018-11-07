
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
		
		//class big_number
		//{
		//	using limb_t = std::uint32_t;
		//	using data_t = std::vector<limb_t>;

		//	constexpr auto limb_digits = std::numeric_limits<limb_t>::digits;
		//	constexpr auto limb_max = std::numeric_limits<limb_t>::max();

		//	data_t m_data;

		//public:

		//	big_number() = default;

		//	template<class numeric_t, typename = std::enable_if_t<std::is_unsigned_v<numeric_t> && std::is_integral_v<numeric_t>>>
		//	explicit big_number(numeric_t n)
		//	{
		//		// ...
		//	}

		//	big_number(big_number const&) = default;
		//	big_number& operator=(big_number const&) = default;

		//	big_number(big_number&&) = default;
		//	big_number& operator=(big_number&&) = default;



		//	// get, set, flip

		//	std::size_t size() const; // returns number of bits

		//	// math operators

		//	// limb stuff?
		//};



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

} // rsa


#include <gtest/gtest.h>

#include <numeric>
#include <algorithm>

namespace test
{

	TEST(Test_RSA, math_to_bits__GivesCorrectNumberOfBits)
	{
		EXPECT_EQ(rsa::math::to_bits(0).size(), 0u);
		EXPECT_EQ(rsa::math::to_bits(1).size(), 1u);
		EXPECT_EQ(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max()).size(), std::numeric_limits<std::uint64_t>::digits);
		EXPECT_EQ(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max() - 1u).size(), std::numeric_limits<std::uint64_t>::digits);
	}

	TEST(Test_RSA, math_to_bits__GivesCorrectBitPattern)
	{
		{
			auto one = std::vector<bool>(1, true);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(1), one));
		}
		{
			auto max = std::vector<bool>(std::numeric_limits<std::uint64_t>::digits, true);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max()), max));
		}
		{
			auto max_sub_one = std::vector<bool>(std::numeric_limits<std::uint64_t>::digits, true);
			max_sub_one[0] = false;
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max() - 1), max_sub_one));
		}
	}

	TEST(Test_RSA, math_to_number__ThrowsWithIncorrectNumberOfBits)
	{
		EXPECT_THROW(rsa::math::to_number(std::vector<bool>(65, false)), std::invalid_argument);
	}

	TEST(Test_RSA, math_to_number__GivesCorrectValue)
	{
		auto digits = std::numeric_limits<std::uint64_t>::digits;
		{
			auto one = std::vector<bool>(digits, false);
			one[0] = true;
			EXPECT_EQ(rsa::math::to_number(one), 1);
		}
		{
			auto max = std::vector<bool>(digits, true);
			EXPECT_EQ(rsa::math::to_number(max), std::numeric_limits<std::uint64_t>::max());
		}
		{
			auto max_sub_one = std::vector<bool>(digits, true);
			max_sub_one[0] = false;
			EXPECT_EQ(rsa::math::to_number(max_sub_one), std::numeric_limits<std::uint64_t>::max() - 1);
		}
	}

	TEST(Test_RSA, math_bits_at__ReturnsBitAtValidIndex)
	{
		auto bits = std::vector<bool>(5, false);
		bits[0] = true;
		bits[4] = true;

		for (auto i = std::size_t{ 0 }; i != bits.size(); ++i)
			EXPECT_EQ(rsa::math::bits_at(bits, i), bits[i]);
	}

	TEST(Test_RSA, math_bits_at__ReturnsFalseAtInvalidIndex)
	{
		auto bits = std::vector<bool>(5, true);
		EXPECT_EQ(rsa::math::bits_at(bits, bits.size()), false);
		EXPECT_EQ(rsa::math::bits_at(bits, std::numeric_limits<std::size_t>::max()), false);
	}

	TEST(Test_RSA, math_bits_equal__IsCorrect)
	{
		{
			auto a = std::vector<bool>(5, false);
			auto b = std::vector<bool>();
			auto c = std::vector<bool>(1, false);
			EXPECT_TRUE(rsa::math::bits_equal(a, b));
			EXPECT_TRUE(rsa::math::bits_equal(b, a));
			EXPECT_TRUE(rsa::math::bits_equal(a, c));
			EXPECT_TRUE(rsa::math::bits_equal(c, a));
			EXPECT_TRUE(rsa::math::bits_equal(c, b));
			EXPECT_TRUE(rsa::math::bits_equal(b, c));
		}
		{
			auto a = std::vector<bool>(5, true);
			auto b = std::vector<bool>();
			auto c = std::vector<bool>(1, true);
			EXPECT_FALSE(rsa::math::bits_equal(a, b));
			EXPECT_FALSE(rsa::math::bits_equal(b, a));
			EXPECT_FALSE(rsa::math::bits_equal(a, c));
			EXPECT_FALSE(rsa::math::bits_equal(c, a));
			EXPECT_FALSE(rsa::math::bits_equal(c, b));
			EXPECT_FALSE(rsa::math::bits_equal(b, c));
		}
		{
			auto a = std::vector<bool>{ true, false, true, false, false };
			auto b = std::vector<bool>{ true, false, true };
			EXPECT_TRUE(rsa::math::bits_equal(a, b));
			EXPECT_TRUE(rsa::math::bits_equal(b, a));
		}
	}

	TEST(Test_RSA, math_bits_greater)
	{
		{
			auto a = std::vector<bool>(5, false);
			auto b = std::vector<bool>();
			auto c = std::vector<bool>(1, false);
			EXPECT_FALSE(rsa::math::bits_greater(a, b));
			EXPECT_FALSE(rsa::math::bits_greater(b, a));
			EXPECT_FALSE(rsa::math::bits_greater(a, c));
			EXPECT_FALSE(rsa::math::bits_greater(c, a));
			EXPECT_FALSE(rsa::math::bits_greater(c, b));
			EXPECT_FALSE(rsa::math::bits_greater(b, c));
		}
		{
			auto a = std::vector<bool>{ true };
			EXPECT_FALSE(rsa::math::bits_greater(a, a));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_greater(a, b));
			EXPECT_FALSE(rsa::math::bits_greater(b, a));
		}
	}

	TEST(Test_RSA, math_bits_not_and__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ };
			auto b = std::vector<bool>{ };
			EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ });
		}
		{
			auto a = std::vector<bool>{ false };
			auto b = std::vector<bool>{ false };
			EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
		}
		{
			auto a = std::vector<bool>{ true };
			auto b = std::vector<bool>{ false };
			EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
		}
		{
			auto a = std::vector<bool>{ true };
			auto b = std::vector<bool>{ true };
			EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ false });
		}
		{
			auto a = std::vector<bool>{ false };
			auto b = std::vector<bool>{ true };
			EXPECT_EQ(rsa::math::bits_not_and(a, b), std::vector<bool>{ true });
		}
	}

	TEST(Test_RSA, math_bits_and__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ false, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{}));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ }));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(a, b), std::vector<bool>{ true, true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_and(b, a), std::vector<bool>{ true, true }));
		}
	}

	TEST(Test_RSA, math_bits_or__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ false, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{}));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true }));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true, true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true, true }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(a, b), std::vector<bool>{ true, true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_or(b, a), std::vector<bool>{ true, true }));
		}
	}

	TEST(Test_RSA, math_bits_xor__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ false, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{}));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ false, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ true }));
		}
		{
			auto a = std::vector<bool>{ true, false };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, false };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ false, true }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ false, true }));
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(a, b), std::vector<bool>{ }));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_xor(b, a), std::vector<bool>{ }));
		}
	}

	TEST(Test_RSA, math_bits_lshift__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ };
			EXPECT_EQ(rsa::math::bits_lshift(a, 0), a);
		}
		{
			auto a = std::vector<bool>{ };
			EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false }));
		}
		{
			auto a = std::vector<bool>{ false };
			EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false, false }));
		}
		{
			auto a = std::vector<bool>{ true };
			EXPECT_EQ(rsa::math::bits_lshift(a, 1), (std::vector<bool>{ false, true }));
		}
	}

	TEST(Test_RSA, math_bits_rshift__IsCorrect)
	{
		{
			auto a = std::vector<bool>{ };
			EXPECT_EQ(rsa::math::bits_rshift(a, 0), a);
		}
		{
			auto a = std::vector<bool>{ };
			EXPECT_EQ(rsa::math::bits_rshift(a, 1), a);
		}
		{
			auto a = std::vector<bool>(4, true);
			EXPECT_EQ(rsa::math::bits_rshift(a, 5), std::vector<bool>{ });
		}
		{
			auto a = std::vector<bool>{ true };
			EXPECT_EQ(rsa::math::bits_rshift(a, 1), std::vector<bool>{ });
		}
		{
			auto a = std::vector<bool>(4, true);
			EXPECT_EQ(rsa::math::bits_rshift(a, 2), std::vector<bool>(2, true));
		}
	}

	TEST(Test_RSA, math_bits_add__IsCorrect)
	{
		{
			auto a = rsa::math::to_bits(0);
			auto b = rsa::math::to_bits(0);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(0)));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(0)));
		}
		{
			auto a = rsa::math::to_bits(1);
			auto b = rsa::math::to_bits(0);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(1)));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(1)));
		}
		{
			auto a = rsa::math::to_bits(1);
			auto b = rsa::math::to_bits(1);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(2)));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(2)));
		}
		{
			auto a = rsa::math::to_bits(273);
			auto b = rsa::math::to_bits(54);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(273 + 54)));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(273 + 54)));
		}
		{
			auto a = rsa::math::to_bits(2734756);
			auto b = rsa::math::to_bits(939876523);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(a, b), rsa::math::to_bits(2734756 + 939876523)));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_add(b, a), rsa::math::to_bits(2734756 + 939876523)));
		}
	}

	TEST(Test_RSA, math_bits_sub__WorksWithValidParameters)
	{
		{
			auto zero = std::vector<bool>{ };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(zero, zero), zero));
		}
		{
			auto one = std::vector<bool>{ true };
			auto zero = std::vector<bool>{ };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(zero, zero), zero));
		}
		{
			auto big = rsa::math::to_bits(53);
			auto smol = rsa::math::to_bits(12);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(big, smol), rsa::math::to_bits(53 - 12)));
		}
		{
			auto big = rsa::math::to_bits(532387);
			auto smol = rsa::math::to_bits(5323);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(big, smol), rsa::math::to_bits(532387 - 5323)));
		}
		{
			auto max = rsa::math::to_bits(std::numeric_limits<std::uint64_t>::max());
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_sub(max, max), std::vector<bool>{ }));
		}
	}

	TEST(Test_RSA, math_bits_sub__ThrowsWithInvalidParameters)
	{
		{
			auto zero = std::vector<bool>{ };
			auto one = std::vector<bool>{ true };
			EXPECT_THROW(rsa::math::bits_sub(zero, one), std::invalid_argument);
		}
		{
			auto a = std::vector<bool>{ true, true };
			auto b = std::vector<bool>{ true, false, true };
			EXPECT_THROW(rsa::math::bits_sub(a, b), std::invalid_argument);
		}
	}

	TEST(Test_RSA, math_bits_mul__IsCorrect)
	{
		{
			auto zero = std::vector<bool>{ };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(zero, zero), zero));
		}
		{
			auto zero = std::vector<bool>{ };
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(zero, one), zero));
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(one, zero), zero));
		}
		{
			auto a = rsa::math::to_bits(53u);
			auto r = rsa::math::to_bits(53u * 53u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(a, a), r));
		}
		{
			auto a = rsa::math::to_bits(923u);
			auto b = rsa::math::to_bits(1u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mul(a, b), a));
		}
	}

	TEST(Test_RSA, math_bits_div__DivisionByZeroThrows)
	{
		{
			auto zero = std::vector<bool>{ };
			EXPECT_THROW(rsa::math::bits_div(zero, zero), std::invalid_argument);
		}
		{
			auto one = std::vector<bool>{ true };
			auto zero = std::vector<bool>{ };
			EXPECT_THROW(rsa::math::bits_div(one, zero), std::invalid_argument);
		}
	}

	TEST(Test_RSA, math_bits_div__WorksWithValidParameters)
	{
		{
			auto zero = std::vector<bool>{ };
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(zero, one), zero));
		}
		{
			auto zero = std::vector<bool>{ };
			auto num = rsa::math::to_bits(1234u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(zero, num), zero));
		}
		{
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(one, one), one));
		}
		{
			auto num = rsa::math::to_bits(1234u);
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(num, num), one));
		}
		{
			auto max = rsa::math::to_bits(4u);
			auto two = rsa::math::to_bits(2u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), two));
		}
		{
			auto max = rsa::math::to_bits(5u);
			auto two = rsa::math::to_bits(2u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), two));
		}
		{
			auto max = rsa::math::to_bits(3u);
			auto two = rsa::math::to_bits(2u);
			auto one = rsa::math::to_bits(1u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_div(max, two), one));
		}
	}

	TEST(Test_RSA, math_bits_mod__ModulusZeroThrows)
	{
		{
			auto zero = std::vector<bool>{ };
			EXPECT_THROW(rsa::math::bits_mod(zero, zero), std::invalid_argument);
		}
		{
			auto one = std::vector<bool>{ true };
			auto zero = std::vector<bool>{ };
			EXPECT_THROW(rsa::math::bits_mod(one, zero), std::invalid_argument);
		}
	}

	TEST(Test_RSA, math_bits_mod__IsCorrect)
	{
		{
			auto zero = std::vector<bool>{ };
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(zero, one), zero));
		}

		{
			auto one = std::vector<bool>{ true };
			auto zero = std::vector<bool>{ };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(one, one), zero));
		}
		{
			auto two = std::vector<bool>{ false, true };
			auto one = std::vector<bool>{ true };
			auto zero = std::vector<bool>{ };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(two, one), zero));
		}
		{
			auto three = std::vector<bool>{ true, true };
			auto two = std::vector<bool>{ false, true };
			auto one = std::vector<bool>{ true };
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(three, two), one));
		}
		{
			auto a = rsa::math::to_bits(342378u);
			auto b = rsa::math::to_bits(448u);
			auto c = rsa::math::to_bits(342378u % 448u);
			EXPECT_TRUE(rsa::math::bits_equal(rsa::math::bits_mod(a, b), c));
		}
	}

	TEST(Test_RSA, math_is_prime_n__IsCorrect)
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

	TEST(Test_RSA, math_is_prime_bits__IsCorrect)
	{
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(0)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(1)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(2)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(3)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(4)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(5)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(6)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(7)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(8)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(9)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(10)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(11)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57047)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57059)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57061)), false);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57073)), true);
		EXPECT_EQ(rsa::math::is_prime(rsa::math::to_bits(57079)), false);
	}

	TEST(Test_RSA, generate_random_bits__SizeOfResultIsCorrect)
	{
		auto rng = std::mt19937_64(std::random_device()());
		EXPECT_EQ(rsa::generate_random_bits(rng, 0).size(), 0);
		EXPECT_EQ(rsa::generate_random_bits(rng, 1).size(), 1);
		EXPECT_EQ(rsa::generate_random_bits(rng, rsa::MAX_RANDOM_BITS).size(), rsa::MAX_RANDOM_BITS);
	}

	TEST(Test_RSA, generate_random_bits__ThrowsIfBitsOutOfRange)
	{
		auto rng = std::mt19937_64(std::random_device()());
		EXPECT_THROW(rsa::generate_random_bits(rng, rsa::MAX_RANDOM_BITS + 1), std::invalid_argument);
		EXPECT_THROW(rsa::generate_random_bits(rng, std::numeric_limits<std::size_t>::max()), std::invalid_argument);
	}

	TEST(Test_RSA, generate_random_bits__ContainsZerosAndOnes)
	{
		auto rng = std::mt19937_64(std::random_device()());
		auto data = rsa::generate_random_bits(rng, 2048);
		EXPECT_FALSE(std::all_of(data.begin(), data.end(), [] (bool b) { return b; }));
		EXPECT_FALSE(std::all_of(data.begin(), data.end(), [] (bool b) { return !b; }));
	}

	TEST(Test_RSA, generate_prime__ThrowsIfBitsOutOfRange)
	{
		EXPECT_THROW(rsa::generate_prime(0), std::invalid_argument);
		EXPECT_THROW(rsa::generate_prime(rsa::MAX_PRIME_BITS + 1), std::invalid_argument);
	}

	TEST(Test_RSA, generate_prime__ReturnsPrime)
	{
		EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(16))));
		EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(24))));
		// too slow... TT
		//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(32))));
		//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(256))));
		//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(512))));
		//EXPECT_TRUE(rsa::math::is_prime(rsa::math::to_number(rsa::generate_prime(1024))));
	}

} // test

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "...";

	return RUN_ALL_TESTS();
}
