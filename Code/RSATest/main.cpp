
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "...";

	return RUN_ALL_TESTS();
}

// TDOO (now):

	// string constructor and to_string()

	// add more tests with 64 bit ints (will break accidental u32 stuff).

	// design:
		// use double block type to improve math operator performance
		// improve multiply, divide, modulus algorithms


// TODO (sometime):
	// construct from biguints with other block sizes
	// fixed size version (whole separate class)
