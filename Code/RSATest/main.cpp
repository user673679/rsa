
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	testing::GTEST_FLAG(filter) = "*math_big_uint__newdiv";

	return RUN_ALL_TESTS();
}


// TODO (now):
	// commit
	// remove divmnu
	// alter divtest to work with div_assign and mod_assign
	// tidy up test cases
	// test new div / mod in benchmarks
	
	// add stuff to utils (db_t(), b_t(), demote -> high_to_low(), promote -> low_to_high(), checked_sub, get_num_leading_zeros)
	// use the utils stuff
	
	// string constructor and to_string()
	// add more tests with 64 bit ints (will break accidental u32 stuff).


// TODO (sometime):
	// construct from biguints with other block sizes
	// fixed size version (whole separate class)
