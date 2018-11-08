#pragma once

namespace rsa
{

	namespace utils
	{

		inline void die()
		{
			__debugbreak();
		}

		inline void die_if(bool condition)
		{
			if (condition)
				die();
		}

	} // utils

} // rsa
