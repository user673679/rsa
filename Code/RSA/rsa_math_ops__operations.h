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
			void bit_and_assign(big_uint<block_t>& a, big_uint<block_t> b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] &= b.data()[i];

				a.data().resize(min_size);
			}

			template<class block_t>
			void bit_and_assign(big_uint<block_t>& a, typename big_uint<block_t>::block_type n)
			{
				if (n == block_t{ 0 })
					a.data().clear();
				else
				{
					a.data().resize(1u);
					a.data().front() &= n;
				}
			}

			template<class block_t>
			void bit_or_assign(big_uint<block_t>& a, big_uint<block_t> b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] |= b.data()[i];

				std::copy(b.data().begin() + min_size, b.data().end(), std::back_inserter(a.data()));
			}

			template<class block_t>
			void bit_or_assign(big_uint<block_t>& a, typename big_uint<block_t>::block_type n)
			{
				if (a.is_zero())
					a.data().push_back(n);
				else
					a.data().front() |= n;
			}

			template<class block_t>
			void bit_xor_assign(big_uint<block_t>& a, big_uint<block_t> b)
			{
				const auto min_size = std::min(a.data().size(), b.data().size());

				for (auto i = std::size_t{ 0 }; i != min_size; ++i)
					a.data()[i] ^= b.data()[i];

				std::copy(b.data().begin() + min_size, b.data().end(), std::back_inserter(a.data()));

				utils::trim(a);
			}

			template<class block_t>
			void bit_xor_assign(big_uint<block_t>& a, typename big_uint<block_t>::block_type n)
			{
				if (a.is_zero())
					a.data().push_back(n);
				else
				{
					a.data().front() ^= n;
					utils::trim(a);
				}
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
				if (b.is_zero())
					return;

				if (a.is_zero())
				{
					a = b;
					return;
				}

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
					const auto b_block = get_block(b.data(), i);
					auto& a_block = get_block_extend(a.data(), i);

					// use bitwise or so both sides are evaluated.
					carry = (checked_addassign(a_block, b_block) | checked_addassign(a_block, carry ? block_t{ 1 } : block_t{ 0 }));
				}

				if (carry)
					a.data().push_back(block_t{ 1 });
			}

			template<class block_t>
			void add_assign(big_uint<block_t>& a, typename big_uint<block_t>::block_type b)
			{
				if (b == block_t{ 0 })
					return;

				if (a.is_zero())
				{
					a.data().push_back(b);
					return;
				}

				const auto checked_addassign = [] (block_t& a, block_t b)
				{
					return ((a += b) < b);
				};

				auto carry = checked_addassign(a.data().front(), b);

				for (auto i = std::size_t{ 1 }; carry && i != a.data().size(); ++i)
					carry = checked_addassign(a.data()[i], block_t{ 1 });

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

				// sub corresponding blocks. in case of underflow, carry one to the next block.
				for (auto i = std::size_t{ 0 }; i != a.data().size(); ++i)
				{
					const auto b_block = get_block(b.data(), i);
					auto& a_block = a.data()[i];

					// use bitwise or so both sides are evaluated.
					borrow = (checked_sub(a_block, a_block, b_block) | checked_sub(a_block, a_block, borrow ? block_t{ 1 } : block_t{ 0 }));
				}

				rsa::utils::die_if(borrow);
				utils::trim(a);
			}

			template<class block_t>
			void sub_assign(big_uint<block_t>& a, typename big_uint<block_t>::block_type b)
			{
				if (b == block_t{ 0 })
					return;

				if (b > a)
					throw std::invalid_argument("cannot subtract larger value from smaller one.");

				const auto checked_sub = [] (block_t& out, block_t a, block_t b)
				{
					return ((out = a - b) > a);
				};

				auto borrow = checked_sub(a.data().front(), a.data().front(), b);

				for (auto i = std::size_t{ 1 }; borrow && i != a.data().size(); ++i)
					borrow = checked_sub(a.data()[i], a.data()[i], block_t{ 1 });

				rsa::utils::die_if(borrow);
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
			void mul_assign(big_uint<block_t>& lhs, typename big_uint<block_t>::block_type rhs)
			{
				if (lhs.is_zero()) return;
				if (rhs == block_t{ 0 }) { lhs.data().clear(); return; }

				if (rhs == 1u) return;
				if (lhs == 1u) { lhs.data().resize(1u); lhs.data().front() = rhs; return; }

				{
					auto b = rhs;
					auto a = std::move(lhs);
					auto& c = lhs;
					rsa::utils::die_if(!c.is_zero());

					while (b != block_t{ 0 })
					{
						if ((b & block_t{ 1 }) != 0u)
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
					const auto d = rhs;
					auto n = std::move(lhs);
					auto& q = lhs;
					rsa::utils::die_if(!q.is_zero());

					auto i = (n.get_most_significant_bit() - d.get_most_significant_bit());
					auto dt = d << i;

					while (n >= d)
					{
						while (dt > n && i != 0u) { --i; dt >>= 1u; }

						q.set_bit(i, true);
						n -= dt;
					}
				}
			}

			template<class block_t>
			void div_assign(big_uint<block_t>& lhs, typename big_uint<block_t>::block_type rhs)
			{
				if (rhs == block_t{ 0 })
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { lhs.data().clear(); return; }
				if (lhs == rhs) { lhs.data().clear(); lhs.data().push_back(block_t{ 1 }); return; }

				auto const get_msb = [] (block_t block)
				{
					if (block == block_t{ 0 })
						throw std::logic_error("number must not be zero.");

					auto count = std::uint8_t{ 0u };
					while (block != block_t{ 1 }) { ++count; block >>= 1u; }
					return count;
				};

				{
					auto const& d = rhs;
					auto n = std::move(lhs);
					auto& q = lhs;
					rsa::utils::die_if(!q.is_zero());

					auto i = (n.get_most_significant_bit() - get_msb(d));
					auto dt = big_uint<block_t>(d) << i;

					while (n >= d)
					{
						while (dt > n && i != 0u) { --i; dt >>= 1u; }

						q.set_bit(i, true);
						n -= dt;
					}
				}
			}

			template<class block_t>
			void mod_assign(big_uint<block_t>& lhs, big_uint<block_t> const& rhs)
			{
				// similar to division, but lhs is the numerator, not the quotient.

				if (rhs.is_zero())
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { return; }
				if (lhs == rhs) { lhs.data().clear(); return; }

				{
					const auto d = rhs;
					auto& n = lhs;
					auto q = big_uint<block_t>();

					auto i = (n.get_most_significant_bit() - d.get_most_significant_bit());
					auto dt = d << i;

					while (n >= d)
					{
						while (dt > n && i != 0u) { --i; dt >>= 1u; }

						q.set_bit(i, true);
						n -= dt;
					}
				}
			}

			template<class block_t>
			void mod_assign(big_uint<block_t>& lhs, typename big_uint<block_t>::block_type rhs)
			{
				if (rhs == block_t{ 0 })
					throw std::invalid_argument("divisor cannot be zero.");

				if (lhs < rhs) { return; }
				if (lhs == rhs) { lhs.data().clear(); return; }

				auto const get_msb = [] (block_t block)
				{
					if (block == block_t{ 0 })
						throw std::logic_error("number must not be zero.");

					auto count = std::uint8_t{ 0u };
					while (block != block_t{ 1 }) { ++count; block >>= 1u; }
					return count;
				};

				{
					auto const& d = rhs;
					auto& n = lhs;
					auto q = big_uint<block_t>();

					auto i = (n.get_most_significant_bit() - get_msb(d));
					auto dt = big_uint<block_t>(d) << i;

					while (n >= d)
					{
						while (dt > n && i != 0u) { --i; dt >>= 1u; }

						q.set_bit(i, true);
						n -= dt;
					}
				}
			}

		} // ops

	} // math

} // rsa
