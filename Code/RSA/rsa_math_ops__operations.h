#pragma once

#include "rsa__utils.h"
#include "rsa_math_meta__type_traits.h"

#include <limits>

namespace rsa
{

	namespace math
	{

		template<class block_t>
		class big_uint;

		namespace ops
		{

			namespace utils
			{

				template<class block_t>
				bool has_extra_empty_blocks(big_uint<block_t> const& a)
				{
					return
						(std::find_if(a.data().rbegin(), a.data().rend(),
							[] (block_t b) { return b != block_t{ 0 }; }).base() !=
							a.data().end());
				}

				template<class block_t>
				void trim(big_uint<block_t>& a)
				{
					a.data().erase(
						std::find_if(a.data().rbegin(), a.data().rend(),
							[] (block_t b) { return b != block_t{ 0 }; }).base(),
						a.data().end());
				}

			} // utils

			template<class block_t>
			void bit_and_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] &= b.data()[i];

				a.data().resize(min_size);
			}

			template<class block_t>
			void bit_or_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] |= b.data()[i];

				std::copy(b.data().begin() + min_size, b.data().end(), std::back_inserter(a.data()));
			}

			template<class block_t>
			void bit_xor_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] ^= b.data()[i];

				std::copy(b.data().begin() + min_size, b.data().end(), std::back_inserter(a.data()));

				utils::trim(a);
			}

			template<class block_t, class uint_t>
			void lshift_assign(big_uint<block_t>& a, uint_t n)
			{
				static_assert(meta::is_uint_v<uint_t>, "shift type must be an unsigned integer.");

				constexpr auto block_digits = meta::digits<block_t>();

				if (n == uint_t{ 0 })
					return;

				if (a.is_zero())
					return;

				// shift by whole blocks
				if (n >= block_digits)
				{
					auto blocks = n / block_digits;
					a.data().insert(a.data().begin(), blocks, block_t{ 0 });

					n -= (blocks * block_digits);

					if (n == uint_t{ 0 })
						return;
				}

				rsa::utils::die_if(n >= block_digits);
				const auto carry_shift = block_t(block_digits - n);
				auto carry = block_t{ 0 };

				// shift by partial blocks
				for (auto& block : a.data())
				{
					// take high bits, shift them to low bits for next block (cast to fix type from integer promotion)
					const auto carry_out = block_t(block >> carry_shift);

					// shift low bits to high, apply carry bits
					block = (block << n) | carry;

					carry = carry_out;
				}

				if (carry != block_t{ 0 })
					a.data().push_back(carry);

				rsa::utils::die_if(utils::has_extra_empty_blocks(a));
			}

			template<class block_t, class uint_t>
			void rshift_assign(big_uint<block_t>& a, uint_t n)
			{
				static_assert(meta::is_uint_v<uint_t>, "shift type must be an unsigned integer.");

				constexpr auto block_digits = meta::digits<block_t>();

				if (n == uint_t{ 0 })
					return;

				if (a.is_zero())
					return;

				// shift by whole blocks
				if (n >= block_digits)
				{
					auto blocks = n / block_digits;
					a.data().erase(a.data().begin(), a.data().begin() + std::min<std::size_t>(blocks, a.data().size()));

					if (a.is_zero())
						return;

					n -= (blocks * block_digits);

					if (n == uint_t{ 0 })
						return;
				}

				rsa::utils::die_if(n >= block_digits);
				const auto carry_shift = block_t(block_digits - n);
				auto carry = block_t{ 0 };

				// shift by partial blocks
				for (auto i_block = a.data().rbegin(); i_block != a.data().rend(); ++i_block)
				{
					auto& block = *i_block;

					// take low bits, shift them to high bits for the next block (cast to fix type from integer promotion)
					const auto carry_out = block_t(block << carry_shift);

					// shift high bits to low, apply carry bits
					block = (block >> n) | carry;

					carry = carry_out;
				}

				utils::trim(a);
			}

			template<class block_t>
			void add_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				if (b.is_zero())
					return;

				using data_type = big_uint<block_t>::data_type;

				const auto get_block = [] (data_type const& data, std::size_t i)
				{
					return (i < data.size()) ? data[i] : block_t{ 0 };
				};

				const auto get_block_extend = [] (data_type& data, std::size_t i) -> block_t&
				{
					if (i == data.size())
						data.push_back(block_t{ 0 });

					return data[i];
				};

				const auto checked_addassign = [] (block_t& a, block_t b)
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
					carry = (checked_addassign(a_block, b_block) | checked_addassign(a_block, carry ? block_t{ 1 } : block_t{ 0 }));
				}

				if (carry)
					a.data().push_back(block_t{ 1 });
			}

			template<class block_t>
			void sub_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				if (b.is_zero())
					return;

				if (b > a)
					throw std::invalid_argument("cannot subtract larger value from smaller one.");

				rsa::utils::die_if(a.data().size() < b.data().size());

				using data_type = big_uint<block_t>::data_type;

				const auto get_block = [] (data_type const& data, std::size_t i)
				{
					return (i < data.size()) ? data[i] : block_t{ 0 };
				};

				const auto checked_sub = [] (block_t& out, block_t a, block_t b)
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
					borrow = (checked_sub(a_block, a_block, b_block) | checked_sub(a_block, a_block, borrow ? block_t{ 1 } : block_t{ 0 }));
				}

				utils::trim(a);
			}

			template<class block_t>
			void mul_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				if (lhs.is_zero()) return;
				if (rhs.is_zero()) { lhs.data().clear(); return; }

				if (rhs == 1u) return;
				if (lhs == 1u) { lhs = rhs; return; }

				{
					// rhs copied before moving from lhs, otherwise lhs *= lhs will break
					auto b = rhs; // copy rhs, so we can modify it
					auto a = std::move(lhs); // move out of lhs (so we can use lhs for the result)
					auto& c = lhs; // put the result in lhs
					rsa::utils::die_if(!c.is_zero());

					while (!b.is_zero())
					{
						if ((b.data()[0] & 1u) != 0u)
							c += a;

						a <<= 1u;
						b >>= 1u;
					}
				}
			}

			template<class block_t>
			void div_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { lhs.data().clear(); return; }
				if (lhs == rhs) { lhs.data().clear(); lhs.data().push_back(1u); return; }

				{
					auto d = rhs;
					auto n = std::move(lhs);
					auto& q = lhs;
					rsa::utils::die_if(!q.is_zero());

					//const auto find_msb = [] (big_uint<block_t> const& m)
					//{
					//	if (m.is_zero())
					//		return std::size_t{ 0u };

					//	auto block = m.data().back();
					//	auto count = std::uint8_t{ 0u };

					//	while (block != block_t{ 0u })
					//	{
					//		++count;
					//		block >>= 1u;
					//	}

					//	return std::size_t{ count + m.data().size() * meta::digits<block_t>() };
					//};

					//d <<= (find_msb(n) - find_msb(d));

					//if (d > n)
					//	d >>= 1u;

					while (n >= d)
					{
						auto i = std::size_t{ 0 };
						auto dt = d;

						while ((n >= (dt << 1u)) && ++i)
							dt <<= 1u;

						q |= (big_uint<block_t>(1u) << i);
						n -= dt;
					}
				}
			}

		} // ops

	} // math

} // rsa
