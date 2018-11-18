
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	//testing::GTEST_FLAG(filter) = "*math_big_uint*";

	return RUN_ALL_TESTS();
}

// TODO (now):
	
	// add stuff to utils (db_t(), b_t(), demote -> high_to_low(), promote -> low_to_high(), checked_sub, get_num_leading_zeros)
	// use the utils stuff
	
// TODO (sometime):
	// construct from biguints with other block sizes
	// fixed size version (entirely separate class)
