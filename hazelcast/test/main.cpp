#include "TestDataSerializable.h"
#include "portableTest.h"
#include "mapTest.h"

int main(int argc, char **argv) {
    try{
//
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

    } catch (hazelcast::client::HazelcastException & exception) {
        std::cout << exception.what() << std::endl;
    } catch(void *s){
        std::cout << s << std::endl;
    }
    return 0;
};


