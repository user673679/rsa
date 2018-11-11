#include "rsa__utils.h"

namespace rsa
{

	namespace utils
	{

		void die()
		{
			__debugbreak();
		}

		void die_if(bool condition)
		{
			if (condition)
				die();
		}

	} // utils

} // rsa
