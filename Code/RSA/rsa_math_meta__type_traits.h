#pragma once

#include <limits>
#include <type_traits>

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

			template<class t>
			constexpr int digits()
			{
				return std::numeric_limits<t>::digits;
			}

			template<class t>
			constexpr t max()
			{
				return std::numeric_limits<t>::max();
			}

		} // meta

	} // math

} // rsa