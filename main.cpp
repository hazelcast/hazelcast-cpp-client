//
// server.cpp
#include "TestDataSerializable.h"
#include "portableTest.h"

using namespace hazelcast::client::serialization;


int main(int argc, char **argv) {
    try{
        testDifferentVersions();
        testRawData();
        testRawDataWithoutRegistering();
        testRawDataInvalidWrite();
        testRawDataInvalidRead();
        testCompression();
        testSerialization();
        testSerializationViaFile();
        testIdentifiedDataSerializable();

//        testBinaryClient();
//        testMapOperations();
//        testMapLocksInSequential();
//        testMapLocksInParallel();
//        testSpeed();
//        testPut();


        std::cout << "Test are completed successfully" << std::endl;
//        std::cin >> argc;

    } catch (hazelcast::client::HazelcastException& exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    return 0;
};


