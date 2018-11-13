#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace rsa
{

	namespace math
	{

		namespace meta
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

			template<class uint_t, class block_t>
			using enable_if_larger_uint_t = std::enable_if_t<is_uint_v<uint_t> && (digits<uint_t>() > digits<block_t>())>;

		} // meta

	} // math

} // rsa
