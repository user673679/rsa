#pragma once

#include <cstdint>
#include <limits>

namespace test
{

	namespace utils
	{

		constexpr auto uint8_max = std::numeric_limits<std::uint8_t>::max();
		constexpr auto uint16_max = std::numeric_limits<std::uint16_t>::max();
		constexpr auto uint32_max = std::numeric_limits<std::uint32_t>::max();
		constexpr auto uint64_max = std::numeric_limits<std::uint64_t>::max();

	} // utils

} // test
