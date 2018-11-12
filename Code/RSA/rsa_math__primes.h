#pragma once

#include "rsa_math__big_uint.h"

#include <array>
#include <cstdint>
#include <random>
#include <set>

namespace rsa
{

	namespace math
	{

		const auto  first_100_primes = std::array<std::uint32_t, 100u>
		{{
			2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
			31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 
			73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 
			127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 
			179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 
			233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 
			283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 
			353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 
			419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 
			467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 
		}};

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

		template<class block_t>
		bool is_prime(math::big_uint<block_t> const& n)
		{
			if (n % 2u == 0u)
				return n == 2u;

			if (n % 3u == 0u)
				return n == 3u;

			for (auto k = math::big_uint<block_t>(6u); (k - 1u) * (k - 1u) <= n; k += 6u)
			{
				if (n % (k - 1u) == 0u || n % (k + 1u) == 0u)
					return false;
			}

			return true;
		}

		template<class block_t>
		bool is_prime_rm(math::big_uint<block_t> const& n, std::mt19937_64& rng, std::size_t k)
		{
			if (n == 0u)
				return false;

			if (n == 1u)
				return true;

			for (auto i = 0u; i != first_100_primes.size(); ++i)
				if (n % first_100_primes[i] == 0u)
					return n == first_100_primes[i];

			auto m = n - 1u;
			auto s = 0u;

			while (m % 2u == 0u) { s += 1u; m /= 2u; }

			auto liars = std::set<math::big_uint<block_t>>();

			auto get_new_x = [&] ()
			{
				auto num_bits = n.get_most_significant_bit();

				while (true)
				{
					auto x = generate_random_bits<block_t>(rng, num_bits);

					if (x > 2u && x < n && liars.count(x) == 0u)
						return x;
				}

				utils::die();
				return math::big_uint<block_t>();
			};

			auto power = [] (math::big_uint<block_t> x, math::big_uint<block_t> m, math::big_uint<block_t> const& n)
			{
				auto a = math::big_uint<block_t>(1u);

				while (m > 0u)
				{
					if (m % 2u == 1u)
						a = (a * x) % n;
					x = (x * x) % n;
					m /= 2u;
				}

				return a;
			};

			while (liars.size() != k)
			{
				auto x = get_new_x();
				auto xi = power(x, m, n);

				auto witness = true;

				if (xi == 1u || xi == n - 1u)
					witness = false;
				else
				{
					for (auto i = math::big_uint<block_t>(0u); i != s - 1u; ++i)
					{
						xi = (xi * xi) % n;

						if (xi == 1u)
							return false;
						else if (xi == n - 1u)
						{
							witness = false;
							break;
						}
					}

					xi = (xi * xi) % n;

					if (xi != 1u)
						return false;
				}

				if (witness)
					return false;
				else
					liars.insert(x);
			}

			return true;
		}

		template<class block_t>
		math::big_uint<block_t> generate_random_bits(std::mt19937_64& rng, std::size_t num_bits)
		{
			auto result = math::big_uint<block_t>();

			if (num_bits == 0u)
				return result;

			auto blocks = num_bits / meta::digits<block_t>();
			result.data().resize(blocks + 1u);
			
			auto dist = std::uniform_int_distribution<block_t>(0u, meta::max<block_t>());

			for (auto& block : result.data())
				block = dist(rng);

			// replace the number in the last block with one that doesn't go over num_bits
			auto final_bits = num_bits - (meta::digits<block_t>() * (result.data().size() - 1u));
			auto final_dist = std::uniform_int_distribution<block_t>(0u, block_t{ 1u } << final_bits);
			result.data().back() = final_dist(rng);

			ops::utils::trim(result);

			return result;
		}

		template<class block_t>
		math::big_uint<block_t> generate_prime(std::mt19937_64& rng, std::size_t num_bits)
		{
			if (num_bits == 0u)
				return{ };

			while (true)
			{
				auto random_bits = generate_random_bits<block_t>(rng, num_bits);

				// set first (ensures odd number) and last (ensures large prime) num_bits
				random_bits.set_bit(0u, true);
				random_bits.set_bit(num_bits - 1u, true);

				auto const two = math::big_uint<block_t>(2u);

				// search for prime number over an interval of 2*num_bits
				auto const search_distance = std::size_t{ 2 } *num_bits;

				for (auto i = std::size_t{ 0 }; i != search_distance; ++i)
				{
					if (math::is_prime(random_bits))
						return random_bits;

					random_bits += two;
				}
			}

			utils::die();
			return{ };
		}

		template<class block_t>
		math::big_uint<block_t> generate_prime_rm(std::mt19937_64& rng, std::size_t num_bits, std::size_t k)
		{
			if (num_bits == 0u)
				return{ };

			while (true)
			{
				auto random_bits = generate_random_bits<block_t>(rng, num_bits);

				// set first (ensures odd number) and last (ensures large prime) num_bits
				random_bits.set_bit(0u, true);
				random_bits.set_bit(num_bits - 1u, true);

				auto const two = math::big_uint<block_t>(2u);

				// search for prime number over an interval of 2*num_bits
				auto const search_distance = 2u * num_bits;

				for (auto i = std::size_t{ 0 }; i != search_distance; ++i)
				{
					if (math::is_prime_rm(random_bits, rng, k))
						return random_bits;

					random_bits += two;
				}
			}

			utils::die();
			return{ };
		}


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

} // rsa