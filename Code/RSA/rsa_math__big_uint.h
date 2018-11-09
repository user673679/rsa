#pragma once

#include "rsa__utils.h"

#include <algorithm>
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

			template<class uint_t>
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

			template<class uint_t>
			uint_t to_uint() const;

			bool is_zero() const;

			data_type& data();
			data_type const& data() const;

#pragma endregion

#pragma region math operators

			big_uint& operator+=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator+=(uint_t n);

			big_uint& operator-=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator-=(uint_t n);

#pragma endregion

		private:

			void trim();

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
		template<class uint_t>
		big_uint<block_t>::big_uint(uint_t n)
		{
			static_assert(meta::is_uint_v<uint_t>, "`uint_t` must be an unsigned integer.");

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
		template<class uint_t>
		uint_t big_uint<block_t>::to_uint() const
		{
			static_assert(meta::is_uint_v<uint_t>, "`uint_t` must be an unsigned integer.");

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
		bool big_uint<block_t>::is_zero() const
		{
			return m_data.empty();
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
			auto& a = *this;

			if (b.is_zero())
				return a;

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

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator-=(big_uint const& b)
		{
			auto& a = *this;

			if (b.is_zero())
				return a;

			if (b > a)
				throw std::invalid_argument("cannot subtract larger value from smaller one.");

			utils::die_if(a.data().size() < b.data().size());

			const auto get_block = [] (data_type const& data, std::size_t i)
			{
				return (i < data.size()) ? data[i] : block_type{ 0 };
			};

			const auto checked_sub = [] (block_type& out, block_type a, block_type b)
			{
				return ((out = a - b) > a);
			};

			auto borrow = false;

			// add corresponding blocks. in case of underflow, carry one to the next block.
			for (auto i = std::size_t{ 0 }; i != a.data().size(); ++i)
			{
				auto const& b_block = get_block(b.data(), i);
				auto& a_block = a.data()[i];

				// use bitwise or so both sides are evaluated.
				borrow = (checked_sub(a_block, a_block, b_block) | checked_sub(a_block, a_block, borrow ? block_type{ 1 } : block_type{ 0 }));
			}

			a.trim();

			return a;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator-=(uint_t n)
		{
			return operator-=(big_uint(n));
		}

#pragma endregion

#pragma region comparison operators

		template<class block_t>
		bool operator==(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return std::equal(a.data().begin(), a.data().end(), b.data().begin(), b.data().end());
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

		template<class block_t>
		bool operator<(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			if (a.data().size() < b.data().size())
				return true;

			if (b.data().size() < a.data().size())
				return false;

			return std::lexicographical_compare(a.data().rbegin(), a.data().rend(), b.data().rbegin(), b.data().rend());
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator<(big_uint<block_t> const& a, uint_t b)
		{
			return (a < big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator<(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) < b);
		}

		template<class block_t>
		bool operator>(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return (b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator>(big_uint<block_t> const& a, uint_t b)
		{
			return (a > big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator>(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) > b);
		}

		template<class block_t>
		bool operator<=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator<=(big_uint<block_t> const& a, uint_t b)
		{
			return (a <= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator<=(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) <= b);
		}

		template<class block_t>
		bool operator>=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(a < b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator>=(big_uint<block_t> const& a, uint_t b)
		{
			return (a >= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		bool operator>=(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) >= b);
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

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator-(big_uint<block_t> a, uint_t b)
		{
			return (a -= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator-(uint_t a, big_uint<block_t> b)
		{
			return (b -= big_uint<block_t>(a));
		}

		template<class block_t>
		big_uint<block_t> operator-(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a -= b);
		}

#pragma endregion

		template<class block_t>
		void big_uint<block_t>::trim()
		{
			m_data.erase(
				std::find_if(m_data.rbegin(), m_data.rend(), 
					[] (block_type b) { return b != block_type{ 0 }; }).base(), 
				m_data.end());
		}

	} // math

} // rsa
