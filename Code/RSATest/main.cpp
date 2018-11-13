
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "...";

	return RUN_ALL_TESTS();
}


// TDOO (now):
	// tidy operation arguments:
		// add routing functions to decide what to do with arguments (if necessary).
	// add more tests with 64 bit ints (will break accidental u32 stuff).

	// design:
		// special versions of operators to deal with block_type uints (just don't accept > block_type values)
		// fixed size version (whole separate class)
		// use double block type to improve math operator performance


// TODO (sometime):
	// construct from biguints with other block sizes
	// string constructor and to_string()
