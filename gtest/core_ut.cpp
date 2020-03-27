#include "gtest/gtest.h"
#include "raylib.h" 

// We expect storing and loading OK in position 0
TEST(SaveStorageValue, test_basic_save_load) {
	int in=19;
	int out=0;
	int position=0;

	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    
    EXPECT_EQ (in,out);
    
}


// We expect to create file storage.data and save/load OK
TEST(SaveStorageValue, create_storage_data_file) {
	int in=19;
	int out=0;
	int position=0;

	system("del storage.data");
	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    EXPECT_EQ (in,out);
}


// We expect to create and increase the size of the file storage.data save/load OK
TEST(SaveStorageValue, increase_storage_data_file) {
	int in=19;
	int out=0;
	int position=0;

	system("del storage.data");
	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    EXPECT_EQ (in,out);
	
	position=23;
	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    EXPECT_EQ (in,out);
  
}


 // We expect failure in SaveStorageValue, and so save is not performed. Load gives different data than save
TEST(SaveStorageValue, test_negative_position) {
	int in=19;
	int out=0;
	int position=-1;
	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    
    EXPECT_NE (in,out);
    
}



  // We expect a failure in SaveStorageValue, and so save is not performed. Load gives different data than save
TEST(SaveStorageValue, fail_realloc_in_SaveStorageValue) {
	int in=19;
	int out=0;
	int position=INT_MAX;
	
	SaveStorageValue(position,in);
	out=LoadStorageValue(position);
    
    EXPECT_NE (in,out);
    
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}