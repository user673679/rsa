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

			template<class block_t>
			void lshift_assign(big_uint<block_t>& a, typename big_uint<block_t>::bit_index_type n)
			{
				using bit_index_t = big_uint<block_t>::bit_index_type;
				constexpr auto block_digits = meta::digits<block_t>();

				if (n == bit_index_t{ 0 })
					return;

				if (a.is_zero())
					return;

				// shift by whole blocks
				if (n >= block_digits)
				{
					auto blocks = n / block_digits;
					a.data().insert(a.data().begin(), blocks, block_t{ 0 });

					n -= (blocks * block_digits);

					if (n == bit_index_t{ 0 })
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

			template<class block_t>
			void rshift_assign(big_uint<block_t>& a, typename big_uint<block_t>::bit_index_type n)
			{
				using bit_index_t = big_uint<block_t>::bit_index_type;
				constexpr auto block_digits = meta::digits<block_t>();

				if (n == bit_index_t{ 0 })
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

					if (n == bit_index_t{ 0 })
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
				using double_block_t = typename big_uint<block_t>::double_block_type;

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
					a_data.push_back(static_cast<block_t>(carry));
			}

			template<class block_t>
			void sub_assign(big_uint<block_t>& a, big_uint<block_t> const& b)
			{
				using double_block_t = typename big_uint<block_t>::double_block_type;

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

			template<class block_t>
			void mul_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				using double_block_t = typename big_uint<block_t>::double_block_type;

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

			template<class block_t>
			void div_test(big_uint<block_t>& quotient, big_uint<block_t>& remainder, big_uint<block_t> dividend, big_uint<block_t> divisor)
			{
				using double_block_t = typename big_uint<block_t>::double_block_type;

				quotient.data().clear();
				remainder.data().clear();

				rsa::utils::die_if(divisor.is_zero());
				rsa::utils::die_if(dividend < divisor);
				rsa::utils::die_if(dividend == divisor);
				rsa::utils::die_if(dividend.data().size() == 1u && divisor.data().size() == 1u);

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
					auto& d = divisor;
					auto& n = remainder;
					remainder = std::move(dividend);
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
						auto n_block = static_cast<double_block_t>(promote(n.data()[j + ds]) | n.data()[j + ds - 1]);
						auto qhat = static_cast<double_block_t>(n_block / v);
						auto rhat = static_cast<double_block_t>(n_block - qhat * v);

						// q is too big or (looking at next block) remainder is smaller than what will be taken away
						while (qhat >= b || (qhat * d.data()[ds - 2]) > (promote(rhat) + n.data()[j + ds - 2]))
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
								auto const p = static_cast<double_block_t>(qhat * d.data()[i]);
								auto const t = static_cast<double_block_t>(n.data()[i + j] - k - static_cast<block_t>(p));
								n.data()[i + j] = static_cast<block_t>(t);
								k = static_cast<double_block_t>(demote(p) - (static_cast<std::make_signed_t<double_block_t>>(t) >> meta::digits<block_t>()));
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
								auto const t = double_block_t{ n.data()[i + j] } +d.data()[i] + k;
								n.data()[i + j] = static_cast<block_t>(t);
								k = static_cast<double_block_t>(t >> meta::digits<block_t>());
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
			void div_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				using double_block_t = typename big_uint<block_t>::double_block_type;

				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { lhs.data().clear(); return; }
				if (lhs == rhs) { lhs.data().clear(); lhs.data().push_back(1); return; }

				if (lhs.data().size() == 1u && rhs.data().size() == 1u)
				{
					lhs = static_cast<block_t>(lhs.data()[0] / rhs.data()[0]);
					return;
				}

				{
					auto q = big_uint<block_t>();
					auto r = big_uint<block_t>();
					div_test(q, r, lhs, rhs);
					lhs = std::move(q);
				}
			}

			template<class block_t>
			void mod_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				using double_block_t = typename big_uint<block_t>::double_block_type;

				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { return; }
				if (lhs == rhs) { lhs.data().clear(); return; }

				if (lhs.data().size() == 1u && rhs.data().size() == 1u)
				{
					lhs = static_cast<block_t>(lhs.data()[0] % rhs.data()[0]);
					utils::trim(lhs);
					return;
				}

				{
					auto q = big_uint<block_t>();
					auto r = big_uint<block_t>();
					div_test(q, r, lhs, rhs);
					lhs = std::move(r);
				}
			}

			template<class block_t>
			void divmod(big_uint<block_t>& quotient, big_uint<block_t>& remainder, big_uint<block_t> const& dividend, big_uint<block_t> const& divisor)
			{
				quotient.data().clear();
				remainder.data().clear();

				if (divisor.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (dividend < divisor) { remainder = dividend; return; }
				if (dividend == divisor) { quotient.data().push_back(1); return; }

				if (dividend.data().size() == 1u && divisor.data().size() == 1u)
				{
					quotient = static_cast<block_t>(dividend.data()[0] / divisor.data()[0]);
					remainder = static_cast<block_t>(dividend.data()[0] % divisor.data()[0]);

					utils::trim(remainder);
					return;
				}

				div_test(quotient, remainder, dividend, divisor);
			}

		} // ops

	} // math

} // rsa
