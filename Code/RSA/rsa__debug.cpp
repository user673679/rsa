#include "rsa__debug.h"

namespace rsa
{

	namespace debug
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

	} // debug

} // rsa
