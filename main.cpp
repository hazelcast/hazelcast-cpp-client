//
// server.cpp
#include <iostream>
#include "ConstantSerializers.h"
#include <cassert>
#include "HazelcastException.h"
#include "TestMobile.h"
#include "SerializationService.h"

using namespace hazelcast::client::serialization;

void testNewDesign();

int main(int argc, char **argv) {
    try{
//        testRawData();
//        testRawDataWithoutRegistering();
//        testRawDataInvalidWrite();
//        testRawDataInvalidRead();
//        testDifferentVersions();
//        testCompression();
//        testSerialization();
//        testSerializationViaFile();

//        testBinaryClient();
//        testMapOperations();
//        testMapLocksInSequential();
//        testMapLocksInParallel();
//        testSpeed();
//        testPut();

        testNewDesign();

        std::cout << "Test are completed successfully" << std::endl;
//        std::cin >> argc;

    } catch (hazelcast::client::HazelcastException exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    return 0;
};


void testNewDesign() {
    SerializationService serializationService(1);
    Data data;
    TestMobile np(4, 'k');
    data = serializationService.toData(np);

    TestMobile tnp1;
    tnp1 = serializationService.toObject<TestMobile >(data);

    assert(np == tnp1);
};