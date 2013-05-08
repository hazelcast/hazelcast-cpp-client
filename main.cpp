//
// server.cpp

#include <iostream>
#include "portableTest.h"
//#include "binaryClientTest.h"

int main(int argc, char **argv) {
    try{
        testRawData();
        testRawDataWithoutRegistering();
        testRawDataInvalidWrite();
        testRawDataInvalidRead();
        testDifferentVersions();
        testCompression();
        testSerialization();
        testSerializationViaFile();

//        testBinaryClient();
//        testMapOperations();
//        testMapLocksInSequential();
//        testMapLocksInParallel();
//        testSpeed();
//        testPut();
        std::cout << "Test are completed successfully" << std::endl;
//        std::cin >> argc;

    } catch (HazelcastException exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    return 0;
};

