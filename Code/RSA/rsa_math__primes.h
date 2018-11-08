#pragma once

namespace rsa
{

	namespace math
	{

		// TODO: move to .cpp
		inline bool is_prime(std::uint64_t n)
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


#pragma region old

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

#pragma endregion

	//math::big_uint_32 generate_random_bits(std::mt19937_64&, std::size_t)
	//{
	//	auto result = math::big_uint_32();

	//	// ...

	//	return result;
	//}

} // rsa