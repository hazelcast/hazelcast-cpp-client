//
// server.cpp

//#include "TestPortableFactory.h"
//#include "SimpleMapTest.h"

#define SERVER_ADDRESS "192.168.2.6:5701"

#include <iostream>
#include "portableTest.h"
//#include "mapTest.h"

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

