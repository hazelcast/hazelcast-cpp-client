#include "TestDataSerializable.h"
#include "portableTest.h"
#include "mapTest.h"
#include <gtest/gtest.h>


TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, 1);
}

GTEST_API_ int main(int argc, char **argv) {
    try{

//        testDifferentVersions();
//        testRawData();
//        testRawDataWithoutRegistering();
//        testRawDataInvalidWrite();
//        testRawDataInvalidRead();
//        testCompression();
//        testSerialization();
//        testSerializationViaFile();
//        testIdentifiedDataSerializable();
//
//        testPutGetRemove();
//        testBinaryClient();
//        testMapOperations();
//        testMapLocksInSequential();
//        testMapLocksInParallel();
        testSpeed();
//        testPut();

        std::cout << "Test are completed successfully" << std::endl;
//        std::cin >> argc;

    } catch (hazelcast::client::HazelcastException & exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};


