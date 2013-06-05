#include "TestDataSerializable.h"
#include "portableTest.h"
#include <mintomic/mintomic.h>
#include <mintpack/timewaster.h>
#include <mintpack/threadsynchronizer.h>


static mint_atomic32_t g_sharedInt;

static void threadFunc(int threadNum) {
    TimeWaster tw(threadNum);

    for (int i = 0; i < 10000000; i++) {
        mint_fetch_add_32_relaxed(&g_sharedInt, 1);
        tw.wasteRandomCycles();
    }
}

bool testFunc(int numThreads) {
    g_sharedInt._nonatomic = 0;
    ThreadSynchronizer threads(numThreads);
    threads.run(threadFunc);
    return g_sharedInt._nonatomic == (uint32_t) 10000000 * numThreads;
}

void *increment(void *x) {
    int *f = (int *) x;
    for (int i = 0; i < 10000; i++)
        __sync_fetch_and_add(f, 1);
    std::cout << ".";
    return NULL;
};

int main(int argc, char **argv) {
    try{
        std::cout << testFunc(2) << std::endl;
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
//        testSpeed();
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


