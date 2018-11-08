#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace rsa
{

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

	} // math

} // rsa
