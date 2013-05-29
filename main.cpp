//
// server.cpp
#include "TestDataSerializable.h"
#include "portableTest.h"
#include "mapTest.h"

using namespace hazelcast::client::serialization;

void *increment(void *x) {
    int *f = (int *) x;
    for (int i = 0; i < 10000; i++)
        __sync_fetch_and_add(f, 1);
    std::cout << ".";

};

int main(int argc, char **argv) {
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

//        testPutGetRemove();
//        testBinaryClient();
//        testMapOperations();
//        testMapLocksInSequential();
//        testMapLocksInParallel();
        testSpeed();
//        testPut();
//        int num = 0;
//
//        for (int i = 0; i < 100; i++) {
//            hazelcast::util::Thread x(increment, &num);
//        }
//
//        sleep(2);
//        std::cout << num << std::endl;
        std::cout << "Test are completed successfully" << std::endl;
//        std::cin >> argc;

    } catch (hazelcast::client::HazelcastException& exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    return 0;
};


