#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace rsa
{

	namespace math
	{

		template<class block_t>
		class big_uint;

		namespace utils
		{

			template<class uint_t>
			constexpr bool is_uint_v = (std::is_integral_v<uint_t> && std::is_unsigned_v<uint_t> && !std::is_same_v<uint_t, bool>);

			template<class uint_t>
			using enable_if_uint_t = std::enable_if_t<is_uint_v<uint_t>>;


			template<class t>
			constexpr std::uint32_t digits()
			{
				return std::uint32_t(std::numeric_limits<t>::digits);
			}

			template<class t>
			constexpr t max()
			{
				return std::numeric_limits<t>::max();
			}


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

	} // math

} // rsa
