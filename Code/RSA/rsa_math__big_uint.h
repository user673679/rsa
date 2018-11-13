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

			big_uint& operator=(block_type n);

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
			big_uint& operator&=(block_type n);

			big_uint& operator|=(big_uint const& b);
			big_uint& operator|=(block_type n);

			big_uint& operator^=(big_uint const& b);
			big_uint& operator^=(block_type n);

			big_uint& operator<<=(bit_index_type n);
			big_uint& operator>>=(bit_index_type n);

#pragma endregion

#pragma region math operators

			big_uint& operator+=(big_uint const& b);
			big_uint& operator+=(block_type n);

			big_uint& operator-=(big_uint const& b);
			big_uint& operator-=(block_type n);

			big_uint& operator*=(big_uint const& b);
			big_uint& operator*=(block_type n);

			big_uint& operator/=(big_uint b);
			big_uint& operator/=(block_type n);

			big_uint& operator%=(big_uint b);
			big_uint& operator%=(block_type n);

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
		big_uint<block_t>& big_uint<block_t>::operator=(block_type n)
		{
			if (n == block_type{ 0 })
				m_data.clear();
			else
			{
				m_data.resize(1u);
				m_data.front() = n;
			}

			return *this;
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
		big_uint<block_t>& big_uint<block_t>::operator&=(block_type n)
		{
			ops::bit_and_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator|=(big_uint const& b)
		{
			ops::bit_or_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator|=(block_type n)
		{
			ops::bit_or_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator^=(big_uint const& b)
		{
			ops::bit_xor_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator^=(block_type n)
		{
			ops::bit_xor_assign(*this, n);
			return *this;
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
		big_uint<block_t>& big_uint<block_t>::operator+=(block_type n)
		{
			ops::add_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator-=(big_uint const& b)
		{
			ops::sub_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator-=(block_type n)
		{
			ops::sub_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator*=(big_uint const& b)
		{
			ops::mul_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator*=(block_type n)
		{
			ops::mul_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator/=(big_uint b)
		{
			ops::div_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator/=(block_type n)
		{
			ops::div_assign(*this, n);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator%=(big_uint b)
		{
			ops::mod_assign(*this, b);
			return *this;
		}

		template<class block_t>
		big_uint<block_t>& big_uint<block_t>::operator%=(block_type n)
		{
			ops::mod_assign(*this, n);
			return *this;
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

		template<class block_t>
		bool operator==(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			return (a.data().empty() && b == block_t{ 0 }) || (a.data().size() == std::size_t{ 1 } && a.data().front() == b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator==(big_uint<block_t> const& a, uint_t b)
		{
			return (a == big_uint<block_t>(b));
		}

		template<class block_t>
		bool operator==(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (b == a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator==(uint_t a, big_uint<block_t> const& b)
		{
			return (b == a);
		}

		template<class block_t>
		bool operator!=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(a == b);
		}

		template<class block_t>
		bool operator!=(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			return !(a == b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator!=(big_uint<block_t> const& a, uint_t b)
		{
			return !(a == b);
		}

		template<class block_t>
		bool operator!=(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return !(a == b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator!=(uint_t a, big_uint<block_t> const& b)
		{
			return !(a == b);
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

		template<class block_t>
		bool operator<(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			if (a.data().size() > std::size_t{ 1 })
				return false;

			if (a.is_zero())
				return (b != block_t{ 0 });

			return (a.data().front() < b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator<(big_uint<block_t> const& a, uint_t b)
		{
			return (a < big_uint<block_t>(b));
		}

		template<class block_t>
		bool operator<(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			if (b.data().size() > std::size_t{ 1 })
				return true;

			if (b.is_zero())
				return false;

			return (a < b.data().front());
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator<(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) < b);
		}

		template<class block_t>
		bool operator>(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return (b < a);
		}

		template<class block_t>
		bool operator>(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			return (b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator>(big_uint<block_t> const& a, uint_t b)
		{
			return (b < a);
		}

		template<class block_t>
		bool operator>(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator>(uint_t a, big_uint<block_t> const& b)
		{
			return (b < a);
		}

		template<class block_t>
		bool operator<=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(b < a);
		}

		template<class block_t>
		bool operator<=(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			return !(b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator<=(big_uint<block_t> const& a, uint_t b)
		{
			return !(b < a);
		}

		template<class block_t>
		bool operator<=(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return !(b < a);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator<=(uint_t a, big_uint<block_t> const& b)
		{
			return !(b < a);
		}

		template<class block_t>
		bool operator>=(big_uint<block_t> const& a, big_uint<block_t> const& b)
		{
			return !(a < b);
		}

		template<class block_t>
		bool operator>=(big_uint<block_t> const& a, typename big_uint<block_t>::block_type b)
		{
			return !(a < b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator>=(big_uint<block_t> const& a, uint_t b)
		{
			return !(a < b);
		}

		template<class block_t>
		bool operator>=(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return !(a < b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		bool operator>=(uint_t a, big_uint<block_t> const& b)
		{
			return !(a < b);
		}

#pragma endregion

#pragma region bitwise operators

		template<class block_t>
		big_uint<block_t> operator&(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a &= b);
		}

		template<class block_t>
		big_uint<block_t> operator&(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a &= b);
		}

		template<class block_t>
		big_uint<block_t> operator|(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a |= b);
		}

		template<class block_t>
		big_uint<block_t> operator|(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a |= b);
		}

		template<class block_t>
		big_uint<block_t> operator^(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a ^= b);
		}

		template<class block_t>
		big_uint<block_t> operator^(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a ^= b);
		}

		template<class block_t>
		big_uint<block_t> operator<<(big_uint<block_t> a, typename big_uint<block_t>::bit_index_type b)
		{
			return (a <<= b);
		}

		template<class block_t>
		big_uint<block_t> operator>>(big_uint<block_t> a, typename big_uint<block_t>::bit_index_type b)
		{
			return (a >>= b);
		}

#pragma endregion

#pragma region math operators

		template<class block_t>
		big_uint<block_t> operator+(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a += b);
		}

		template<class block_t>
		big_uint<block_t> operator+(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a += b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator+(big_uint<block_t> a, uint_t b)
		{
			return (a += big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator+(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) += b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator+(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) += b);
		}

		template<class block_t>
		big_uint<block_t> operator-(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a -= b);
		}

		template<class block_t>
		big_uint<block_t> operator-(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a -= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator-(big_uint<block_t> a, uint_t b)
		{
			return (a -= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator-(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) -= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator-(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) -= b);
		}

		template<class block_t>
		big_uint<block_t> operator*(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a *= b);
		}

		template<class block_t>
		big_uint<block_t> operator*(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a *= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator*(big_uint<block_t> a, uint_t b)
		{
			return (a *= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator*(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) *= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator*(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) *= b);
		}

		template<class block_t>
		big_uint<block_t> operator/(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a /= b);
		}

		template<class block_t>
		big_uint<block_t> operator/(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a /= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator/(big_uint<block_t> a, uint_t b)
		{
			return (a /= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator/(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) /= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator/(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) /= b);
		}

		template<class block_t>
		big_uint<block_t> operator%(big_uint<block_t> a, big_uint<block_t> const& b)
		{
			return (a %= b);
		}

		template<class block_t>
		big_uint<block_t> operator%(big_uint<block_t> a, typename big_uint<block_t>::block_type b)
		{
			return (a %= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator%(big_uint<block_t> a, uint_t b)
		{
			return (a %= big_uint<block_t>(b));
		}

		template<class block_t>
		big_uint<block_t> operator%(typename big_uint<block_t>::block_type a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) %= b);
		}

		template<class block_t, class uint_t, typename = meta::enable_if_larger_uint_t<uint_t, block_t>>
		big_uint<block_t> operator%(uint_t a, big_uint<block_t> const& b)
		{
			return (big_uint<block_t>(a) %= b);
		}

#pragma endregion

	} // math

} // rsa
