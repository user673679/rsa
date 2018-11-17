#pragma once

#include "rsa__utils.h"
#include "rsa_math_meta__type_traits.h"
#include "rsa_math_ops__operations.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <iostream>

int divmnu(unsigned short q[], unsigned short r[], const unsigned short u[], const unsigned short v[], int m, int n);

namespace rsa
{

	namespace math
	{

		template<class block_t>
		class big_uint
		{
		public:

			static_assert(meta::is_uint_v<block_t>, "`uint_t` must be an unsigned integer.");

			using block_type = block_t;
			using data_type = std::vector<block_type>;
			using bit_index_type = std::size_t;

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

			bool get_bit(bit_index_type i) const;
			void set_bit(bit_index_type i, bool value);
			void flip_bit(bit_index_type i);

			bit_index_type get_most_significant_bit() const;

			data_type& data();
			data_type const& data() const;

#pragma endregion

#pragma region bitwise operators

			big_uint& operator&=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator&=(uint_t n);

			big_uint& operator|=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator|=(uint_t n);

			big_uint& operator^=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator^=(uint_t n);

			big_uint& operator<<=(bit_index_type n);

			big_uint& operator>>=(bit_index_type n);

#pragma endregion

#pragma region math operators

			big_uint& operator+=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator+=(uint_t n);

			big_uint& operator-=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator-=(uint_t n);

			big_uint& operator*=(big_uint const& b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator*=(uint_t n);

			big_uint& operator/=(big_uint b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator/=(uint_t n);

			big_uint& operator%=(big_uint b);

			template<class uint_t, typename = meta::enable_if_uint_t<uint_t>>
			big_uint& operator%=(uint_t n);

			big_uint& operator++();
			big_uint operator++(int);

			big_uint& operator--();
			big_uint operator--(int);

#pragma endregion

		private:

			data_type m_data;
		};

		using big_uint_8 = big_uint<std::uint8_t>;
		using big_uint_16 = big_uint<std::uint16_t>;
		using big_uint_32 = big_uint<std::uint32_t>;
		using big_uint_64 = big_uint<std::uint64_t>;

#pragma region members - construct

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
			constexpr bool can_rshift = (meta::digits<block_type>() < meta::digits<uint_t>());

			while (n != uint_t{ 0 })
			{
				// integer promotion, conversion to greater rank, implicit conversion to block_type
				m_data.push_back(meta::max<block_type>() & n);

				if (can_rshift)
					n >>= meta::digits<block_type>();
				else
					n = uint_t{ 0 };
			}
		}

#pragma endregion

#pragma region members - general

		template<class block_t>
		template<class uint_t>
		uint_t big_uint<block_t>::to_uint() const
		{
			static_assert(meta::is_uint_v<uint_t>, "`uint_t` must be an unsigned integer.");

			// it's much easier to static_assert / throw here if uint_t may be too small.
			// checking the actual value would be a lot more work.
			{
				static_assert(meta::digits<block_t>() <= meta::digits<uint_t>(), "uint_t may be too small to represent this number.");

				if (m_data.size() * meta::digits<block_type>() > meta::digits<uint_t>())
					throw std::range_error("uint_t may be too small to represent this number.");
			}

			auto result = uint_t{ 0 };

			if (m_data.empty())
				return result;

			for (auto i = std::size_t{ 0 }; i != m_data.size(); ++i)
				result |= (uint_t{ m_data[i] } << (i * meta::digits<block_type>()));

			return result;
		}

		template<class block_t>
		bool big_uint<block_t>::is_zero() const
		{
			return m_data.empty();
		}

		template<class block_t>
		bool big_uint<block_t>::get_bit(bit_index_type i) const
		{
			auto block_index = i / meta::digits<block_type>();

			if (m_data.size() <= block_index)
				return false;

			auto block_bit = i - (block_index * meta::digits<block_type>());
			utils::die_if(block_bit >= meta::digits<block_type>());

			return bool((m_data[block_index] >> block_bit) & 1u);
		}

		template<class block_t>
		void big_uint<block_t>::set_bit(bit_index_type i, bool value)
		{
			auto block_index = i / meta::digits<block_type>();

			if (m_data.size() <= block_index)
				m_data.resize(block_index + 1u);

			auto block_bit = i - (block_index * meta::digits<block_type>());
			utils::die_if(block_bit >= meta::digits<block_type>());

			auto mask = block_type(block_type{ 1u } << block_bit);
			m_data[block_index] |= mask & block_type(block_type{ value } << block_bit);
		}

		template<class block_t>
		void big_uint<block_t>::flip_bit(bit_index_type i)
		{
			auto block_index = i / meta::digits<block_type>();

			if (m_data.size() <= block_index)
				m_data.resize(block_index + 1u);

			auto block_bit = i - (block_index * meta::digits<block_type>());
			utils::die_if(block_bit >= meta::digits<block_type>());

			auto mask = block_type(block_type{ 1u } << block_bit);
			m_data[block_index] ^= mask;
		}

		template<class block_t>
		typename big_uint<block_t>::bit_index_type big_uint<block_t>::get_most_significant_bit() const
		{
			if (is_zero())
				throw std::logic_error("number must not be zero.");

			auto block = m_data.back();
			auto count = std::uint8_t{ 0u };

			while (block != block_type{ 1u })
			{
				++count;
				block >>= 1u;
			}

			return bit_index_type{ count + (m_data.size() - 1u) * meta::digits<block_type>() };
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

#pragma region members - assignment

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator=(uint_t n)
		{
			return (*this = big_uint(n));
		}

#pragma endregion

#pragma region members - bitwise operators

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator&=(big_uint const& b)
		{
			ops::bit_and_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator&=(uint_t n)
		{
			return operator&=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator|=(big_uint const& b)
		{
			ops::bit_or_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator|=(uint_t n)
		{
			return operator|=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator^=(big_uint const& b)
		{
			ops::bit_xor_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator^=(uint_t n)
		{
			return operator^=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator<<=(bit_index_type n)
		{
			ops::lshift_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator>>=(bit_index_type n)
		{
			ops::rshift_assign(*this, n);
			return *this;
		}

#pragma endregion

#pragma region members - math operators

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator+=(big_uint const& b)
		{
			ops::add_assign(*this, b);
			return *this;
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
			ops::sub_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator-=(uint_t n)
		{
			return operator-=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator*=(big_uint const& b)
		{
			ops::mul_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator*=(uint_t n)
		{
			return operator*=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator/=(big_uint b)
		{
			ops::div_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator/=(uint_t n)
		{
			return operator/=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator%=(big_uint b)
		{
			ops::mod_assign(*this, b);
			return *this;
		}

		template<class block_t>
		template<class uint_t, typename>
		big_uint<block_t>& big_uint<block_t>::operator%=(uint_t n)
		{
			return operator%=(big_uint(n));
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator++()
		{
			return operator+=(1u);
		}

		template<class block_t>
		big_uint<block_t> big_uint<block_t>::operator++(int)
		{
			auto temp = *this;
			operator++();
			return temp;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator--()
		{
			return operator-=(1u);
		}

		template<class block_t>
		big_uint<block_t> big_uint<block_t>::operator--(int)
		{
			auto temp = *this;
			operator--();
			return temp;
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

#pragma region bitwise operators

		template<class block_t>
		big_uint<block_t> operator&(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a &= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator&(big_uint<block_t> a, uint_t b)
		{
			return (a &= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator|(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a |= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator|(big_uint<block_t> a, uint_t b)
		{
			return (a |= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator^(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a ^= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator^(big_uint<block_t> a, uint_t b)
		{
			return (a ^= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator<<(big_uint<block_t> a, uint_t b)
		{
			return (a <<= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator>>(big_uint<block_t> a, uint_t b)
		{
			return (a >>= b);
		}

#pragma endregion

#pragma region math operators

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator+(big_uint<block_t> a, uint_t b)
		{
			return (a += big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator+(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) += b);
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
		big_uint<block_t> operator-(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) -= b);
		}

		template<class block_t>
		big_uint<block_t> operator-(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a -= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator*(big_uint<block_t> a, uint_t b)
		{
			return (a *= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator*(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) *= b);
		}

		template<class block_t>
		big_uint<block_t> operator*(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a *= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator/(big_uint<block_t> a, uint_t b)
		{
			return (a /= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator/(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) /= b);
		}

		template<class block_t>
		big_uint<block_t> operator/(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a /= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator%(big_uint<block_t> a, uint_t b)
		{
			return (a %= big_uint<block_t>(b));
		}

		template<class block_t, class uint_t, typename = meta::enable_if_uint_t<uint_t>>
		big_uint<block_t> operator%(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) %= b);
		}

		template<class block_t>
		big_uint<block_t> operator%(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a %= b);
		}

#pragma endregion

		namespace ops
		{

			template<>
			inline void add_assign(big_uint<std::uint32_t>& a, big_uint<std::uint32_t> const& b)
			{
				using block_t = std::uint32_t;
				using double_block_t = std::uint64_t;

				if (b.is_zero())
					return;

				if (a.is_zero())
				{
					a = b;
					return;
				}

				auto& a_data = a.data();
				auto const& b_data = b.data();
				const auto min_size = std::min(a_data.size(), b_data.size());

				auto carry = double_block_t{ 0 };

				// both a and b have data
				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
				{
					carry += static_cast<double_block_t>(a_data[i]) + static_cast<double_block_t>(b_data[i]);
					a_data[i] = static_cast<block_t>(carry);
					carry >>= meta::digits<block_t>();
				}

				// ran out of data in a, copy over the rest of b
				a_data.insert(a_data.end(), b_data.begin() + min_size, b_data.end());

				// add carry
				for (auto i = min_size; i != a_data.size() && (carry != double_block_t{ 0 }); ++i)
				{
					carry += static_cast<double_block_t>(a_data[i]);
					a_data[i] = static_cast<block_t>(carry);
					carry >>= meta::digits<block_t>();
				}

				// extend a if necessary
				if (carry)
					a_data.push_back(static_cast<std::uint32_t>(carry));
			}

			template<>
			inline void sub_assign(big_uint<std::uint32_t>& a, big_uint<std::uint32_t> const& b)
			{
				using block_t = std::uint32_t;
				using double_block_t = std::uint64_t;

				if (b.is_zero())
					return;

				if (b > a)
					throw std::invalid_argument("cannot subtract larger value from smaller one.");

				rsa::utils::die_if(a.data().size() < b.data().size());

				auto& a_data = a.data();
				auto const& b_data = b.data();

				auto borrow = double_block_t{ 0 };

				// both a and b have data
				for (auto i = std::size_t{ 0 }; i != b_data.size(); ++i)
				{
					borrow = static_cast<double_block_t>(a_data[i]) - static_cast<double_block_t>(b_data[i]) - borrow;
					a_data[i] = static_cast<block_t>(borrow);
					borrow = (borrow >> meta::digits<block_t>()) & double_block_t { 1 };
				}

				// ran out of data in b, subtract borrow
				for (auto i = b_data.size(); i != a_data.size() && (borrow != double_block_t{ 0 }); ++i)
				{
					borrow = static_cast<double_block_t>(a_data[i]) - borrow;
					a_data[i] = static_cast<block_t>(borrow);
					borrow = (borrow >> meta::digits<block_t>()) & double_block_t { 1 };
				}

				utils::trim(a);
			}

			template<>
			inline void mul_assign(big_uint<std::uint32_t>& lhs, big_uint<std::uint32_t> const& rhs)
			{
				using block_t = std::uint32_t;
				using double_block_t = std::uint64_t;

				if (lhs.is_zero()) return;
				if (rhs.is_zero()) { lhs.data().clear(); return; }

				if (rhs == 1u) return;
				if (lhs == 1u) { lhs = rhs; return; }

				// note: long multiplication relies on:
				// double_block_t holding (max<block_t>() * max<block_t>() + 2 * max<block_t>())
				// which is exactly the case if digits<double_block_t>() == 2 * digits<block_t>();

				{
					auto b = rhs; // TODO: we only need to copy this if lhs and rhs refer to the same object
					auto a = std::move(lhs);
					auto& c = lhs;

					auto const& a_data = a.data();
					auto const& b_data = b.data();
					auto& c_data = c.data();

					c_data.resize(a_data.size() + b_data.size());

					for (auto i = std::size_t{ 0 }; i != a_data.size(); ++i)
					{
						auto carry = double_block_t{ 0 };

						for (auto j = std::size_t{ 0 }; j != b_data.size(); ++j)
						{
							carry += static_cast<double_block_t>(a_data[i]) * static_cast<double_block_t>(b_data[j]);
							carry += c_data[i + j];
							c_data[i + j] = static_cast<block_t>(carry);
							carry >>= meta::digits<block_t>();
						}

						// c_data[i + b_data.size()] is always zero
						if (carry)
							c_data[i + b_data.size()] = static_cast<block_t>(carry);
					}

					utils::trim(c);
				}
			}

			template<class block_t, class double_block_t>
			inline void div_test(big_uint<block_t>& quotient, big_uint<block_t>& remainder, big_uint<block_t> const& lhs, big_uint<block_t> const& rhs)
			{
				quotient.data().clear();
				remainder.data().clear();

				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { remainder = lhs; quotient.data().clear(); return; }
				if (lhs == rhs) { quotient.data().clear(); quotient.data().push_back(1); return; }

				if (lhs.data().size() == 1u && rhs.data().size() == 1u)
				{
					quotient = static_cast<block_t>(lhs.data()[0] / rhs.data()[0]);
					remainder = static_cast<block_t>(lhs.data()[0] % rhs.data()[0]);

					utils::trim(remainder);
					return;
				}

				auto const get_num_leading_zeros = [] (block_t x)
				{
					rsa::utils::die_if(x == 0);

					auto count = std::size_t{ 0 };

					while (x != 0)
					{
						++count;
						x >>= 1;
					}

					return meta::digits<block_t>() - count;
				};

				auto const promote = [] (double_block_t b) { return b << meta::digits<block_t>(); };
				auto const demote = [] (double_block_t b) { return b >> meta::digits<block_t>(); };
				auto const checked_sub = [] (block_t& out, block_t a, block_t b) {
					return ((out = a - b) > a);
				};

				{
					auto d = rhs;
					auto& n = remainder;
					remainder = lhs;
					auto& q = quotient;

					q.data().resize(n.data().size() - d.data().size() + 1);

					// single digit divisor
					if (d.data().size() == 1)
					{
						auto k = double_block_t{ 0 };
						auto const v = d.data()[0];

						for (auto i = n.data().size(); i != 0; --i)
						{
							auto const index = i - 1;
							k = (k << meta::digits<block_t>()) + n.data()[index];
							q.data()[index] = static_cast<block_t>(k / v);
							k -= static_cast<double_block_t>(q.data()[index]) * v;
						}

						n.data().clear();

						if (k != 0)
							n.data().push_back(static_cast<block_t>(k));

						utils::trim(q);

						return;
					}

					auto const b = double_block_t{ 1 } << meta::digits<block_t>();
					auto const ns = n.data().size(); // m
					auto const ds = d.data().size(); // n

					auto shift = get_num_leading_zeros(d.data().back());
					d <<= shift;
					n <<= shift;

					if (n.data().size() == ns)
						n.data().push_back(block_t{ 0 });

					for (auto i_outer = (ns - ds) + 1; i_outer != 0; --i_outer)
					{
						auto const j = i_outer - 1;

						// take the top two blocks of n, divide by top block of d, calc remainder
						auto v = d.data()[ds - 1];
						auto n_block = promote(n.data()[j + ds]) | n.data()[j + ds - 1];
						auto qhat = n_block / v;
						auto rhat = n_block - qhat * v;

						// q is too big or (looking at next block) remainder is smaller than what will be taken away
						while (qhat >= b || (qhat * d.data()[ds - 2]) > promote(rhat) + n.data()[j + ds - 2])
						{
							qhat -= 1; rhat += v;
							if (rhat >= b) break;
						}

						// qhat is now correct, or 1 too high (extremely rare)

						// multiply divisor by qhat and subtract from n
						auto underflow = false;
						{
							auto k = double_block_t{ 0 };
							for (auto i = std::size_t{ 0 }; i != ds; ++i)
							{
								auto const p = qhat * d.data()[i];
								auto const t = n.data()[i + j] - k - static_cast<block_t>(p);
								n.data()[i + j] = static_cast<block_t>(t);
								k = demote(p) - (static_cast<std::make_signed_t<double_block_t>>(t) >> meta::digits<block_t>());
							}

							if (k != 0)
								underflow |= checked_sub(n.data()[j + ds], n.data()[j + ds], static_cast<block_t>(k));
						}

						// set quotient
						q.data()[j] = static_cast<block_t>(qhat);

						// underflow! (qhat was 1 too high)
						// decrement q and add back one divisor to the remainder
						if (underflow)
						{
							q.data()[j] = q.data()[j] - 1;

							auto k = double_block_t{ 0 };
							for (auto i = std::size_t{ 0 }; i != ds; ++i)
							{
								k += n.data()[i + j] + d.data()[i];
								n.data()[i + j] = static_cast<block_t>(k);
								k >>= meta::digits<block_t>();
							}

							n.data()[j + ds] += static_cast<block_t>(k);
						}
					}

					utils::trim(q);

					// shift remainder back
					utils::trim(n);
					n >>= shift;
				}
			}

			template<class block_t>
			void pvec(std::vector<block_t> const& vec)
			{
				std::cout << "{ ";
				for (auto v : vec)
					std::cout << v << ", ";
				std::cout << "}, ";
			}

			inline int call_divmnu(rsa::math::big_uint_16& q, rsa::math::big_uint_16& r, rsa::math::big_uint_16 const& a, rsa::math::big_uint_16 const& b)
			{
				if (b.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (a < b) { r = a; q.data().clear(); return 0; }
				if (a == b) { q.data().clear(); q.data().push_back(1); return 0; }

				if (a.data().size() == 1u && b.data().size() == 1u)
				{
					q = static_cast<std::uint16_t>(a.data()[0] / b.data()[0]);
					r = static_cast<std::uint16_t>(a.data()[0] % b.data()[0]);

					utils::trim(r);
					return 0;
				}

				auto m = static_cast<int>(a.data().size());
				auto n = static_cast<int>(b.data().size());
				q.data().resize(std::max(m - n + 1, 1));
				r.data().resize(n);

				auto result = divmnu(q.data().data(), r.data().data(), a.data().data(), b.data().data(), m, n);

				rsa::math::ops::utils::trim(q);
				rsa::math::ops::utils::trim(r);
				return result;
			}

			template<>
			inline void div_assign(big_uint<std::uint32_t>& lhs, big_uint<std::uint32_t> const& rhs)
			{
				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { lhs.data().clear(); return; }
				if (lhs == rhs) { lhs.data().clear(); lhs.data().push_back(1); return; }

				auto q1 = big_uint<std::uint32_t>();
				auto r1 = big_uint<std::uint32_t>();
				divmod(q1, r1, lhs, rhs);

				auto q2 = big_uint<std::uint32_t>();
				auto r2 = big_uint<std::uint32_t>();
				div_test<std::uint32_t, std::uint64_t>(q2, r2, lhs, rhs);

				if (q2 != q1 || r2 != r1)
				{
					std::cout << std::hex;
					std::cout << "div: ";
					std::cout << "{ ";
					pvec(lhs.data());
					pvec(rhs.data());
					pvec(q1.data());
					pvec(r1.data());
					std::cout << "},\n";
				}

				lhs = q1;
			}

			template<>
			inline void mod_assign(big_uint<std::uint32_t>& lhs, big_uint<std::uint32_t> const& rhs)
			{
				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { return; }
				if (lhs == rhs) { lhs.data().clear(); return; }


				auto q1 = big_uint<std::uint32_t>();
				auto r1 = big_uint<std::uint32_t>();
				divmod(q1, r1, lhs, rhs);

				//auto q1 = big_uint<std::uint16_t>();
				//auto r1 = big_uint<std::uint16_t>();
				//rsa::utils::die_if(call_divmnu(q1, r1, lhs, rhs) != 0);

				auto q2 = big_uint<std::uint32_t>();
				auto r2 = big_uint<std::uint32_t>();
				div_test<std::uint32_t, std::uint64_t>(q2, r2, lhs, rhs);

				if (q2 != q1 || r2 != r1)
				{
					std::cout << std::hex;
					std::cout << "div: ";
					std::cout << "{ ";
					pvec(lhs.data());
					pvec(rhs.data());
					pvec(q1.data());
					pvec(r1.data());
					std::cout << "},\n";
				}

				lhs = r1;
			}

		} // ops

	} // math

} // rsa


#include <stdio.h>
#include <stdlib.h>     //To define "exit", req'd by XLC.
#include <malloc.h>

#pragma warning(push)
#pragma warning(disable: 4244 4554)

#define max(x, y) ((x) > (y) ? (x) : (y))

inline int nlz(unsigned x) {
	int n;

	if (x == 0) return(32);
	n = 0;
	if (x <= 0x0000FFFF) { n = n + 16; x = x << 16; }
	if (x <= 0x00FFFFFF) { n = n + 8; x = x << 8; }
	if (x <= 0x0FFFFFFF) { n = n + 4; x = x << 4; }
	if (x <= 0x3FFFFFFF) { n = n + 2; x = x << 2; }
	if (x <= 0x7FFFFFFF) { n = n + 1; }
	return n;
}

inline int divmnu(unsigned short q[], unsigned short r[], const unsigned short u[], const unsigned short v[], int m, int n) {

	const unsigned b = 65536; // Number base (16 bits).
	unsigned short *un, *vn;  // Normalized form of u, v.
	unsigned qhat;            // Estimated quotient digit.
	unsigned rhat;            // A remainder.
	unsigned p;               // Product of two digits.
	int s, i, j, t, k;

	if (m < n || n <= 0 || v[n - 1] == 0)
		return 1;              // Return if invalid param.

	if (n == 1) {                        // Take care of
		k = 0;                            // the case of a
		for (j = m - 1; j >= 0; j--) {    // single-digit
			q[j] = (k*b + u[j]) / v[0];      // divisor here.
			k = (k*b + u[j]) - q[j] * v[0];
		}
		if (r != NULL) r[0] = k;
		return 0;
	}

	// Normalize by shifting v left just enough so that
	// its high-order bit is on, and shift u left the
	// same amount.  We may have to append a high-order
	// digit on the dividend; we do that unconditionally.

	s = nlz(v[n - 1]) - 16;        // 0 <= s <= 15.
	vn = (unsigned short *)_alloca(2 * n);
	for (i = n - 1; i > 0; i--)
		vn[i] = (v[i] << s) | (v[i - 1] >> 16 - s);
	vn[0] = v[0] << s;

	un = (unsigned short *)_alloca(2 * (m + 1));
	un[m] = u[m - 1] >> 16 - s;
	for (i = m - 1; i > 0; i--)
		un[i] = (u[i] << s) | (u[i - 1] >> 16 - s);
	un[0] = u[0] << s;

	for (j = m - n; j >= 0; j--) {       // Main loop.
										 // Compute estimate qhat of q[j].
		qhat = (un[j + n] * b + un[j + n - 1]) / vn[n - 1];
		rhat = (un[j + n] * b + un[j + n - 1]) - qhat*vn[n - 1];
	again:
		if (qhat >= b || qhat*vn[n - 2] > b*rhat + un[j + n - 2])
		{
			qhat = qhat - 1;
			rhat = rhat + vn[n - 1];
			if (rhat < b) goto again;
		}

		// Multiply and subtract.
		k = 0;
		for (i = 0; i < n; i++) {
			p = qhat*vn[i];
			t = un[i + j] - k - (p & 0xFFFF);
			un[i + j] = t;
			k = (p >> 16) - (t >> 16);
		}
		t = un[j + n] - k;
		un[j + n] = t;

		q[j] = qhat;              // Store quotient digit.
		if (t < 0) {              // If we subtracted too
			q[j] = q[j] - 1;       // much, add back.
			k = 0;
			for (i = 0; i < n; i++) {
				t = un[i + j] + vn[i] + k;
				un[i + j] = t;
				k = t >> 16;
			}
			un[j + n] = un[j + n] + k;
		}
	} // End j.
	  // If the caller wants the remainder, unnormalize
	  // it and pass it back.
	if (r != NULL) {
		for (i = 0; i < n; i++)
			r[i] = (un[i] >> s) | (un[i + 1] << 16 - s);
	}
	return 0;
}

#undef max

#pragma warning(pop)
