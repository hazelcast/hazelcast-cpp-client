#include "SimpleMapTest.h"
#include "HazelcastServerFactory.h"
#include "queue/ClientQueueTest.h"
#include "multimap/ClientMultiMapTest.h"
#include "map/ClientMapTest.h"
#include "serialization/ClientSerializationTest.h"
#include "list/ClientListTest.h"
#include "set/ClientSetTest.h"
#include "atomiclong/IAtomicLongTest.h"
#include "topic/ClientTopicTest.h"
#include "idgenerator/IdGeneratorTest.h"
#include "countdownlatch/ICountDownLatchTest.h"
#include "lock/ClientLockTest.h"
#include "semaphore/ClientSemaphoreTest.h"
#include "txn/ClientTxnTest.h"
#include "txn/ClientTxnSetTest.h"
#include "txn/ClientTxnQueueTest.h"
#include "txn/ClientTxnMapTest.h"
#include "txn/ClientTxnListTest.h"
#include "txn/ClientTxnMultiMapTest.h"
#include "cluster/ClusterTest.h"
#include "cluster/MemberAttributeTest.h"
#include "issues/IssueTest.h"
#include "util/ClientUtilTest.h"
#include "license/extractor/LicenseExtractorTest.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)


void handler(int sig) {
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    exit(1);
}

#else


#include <execinfo.h>
#include <unistd.h>

void handler(int sig) {
    void *array[20];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 20);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

#endif

using namespace hazelcast::client::test;

void testSpeed(const char* address) {
    SimpleMapTest s(address, 5701);
    s.run();
}

int unitTests(const char* address) {
    try {
        RUN_TEST(ClientUtilTest, 1);
        RUN_TEST_NO_ARGS(license::LicenseExtractorTest);
        RUN_TEST(ClientSerializationTest, 1);
        HazelcastServerFactory factory(address);
        RUN_TEST(ClientMapTest, factory);
        RUN_TEST(IssueTest, factory);
        RUN_TEST(MemberAttributeTest, factory);
        RUN_TEST(ClusterTest, factory);
        RUN_TEST(ClientMultiMapTest, factory);
        RUN_TEST(ClientQueueTest, factory);
        RUN_TEST(ClientListTest, factory);
        RUN_TEST(ClientSetTest, factory);
        RUN_TEST(IAtomicLongTest, factory);
        RUN_TEST(IdGeneratorTest, factory);
        RUN_TEST(ICountDownLatchTest, factory);
        RUN_TEST(ClientLockTest, factory);
        RUN_TEST(ClientSemaphoreTest, factory);
        RUN_TEST(ClientTopicTest, factory);
        RUN_TEST(ClientTxnListTest, factory);
        RUN_TEST(ClientTxnMapTest, factory);
        RUN_TEST(ClientTxnMultiMapTest, factory);
        RUN_TEST(ClientTxnQueueTest, factory);
        RUN_TEST(ClientTxnSetTest, factory);
        RUN_TEST(ClientTxnTest, factory);
        return 0;
    } catch (std::exception& e) {
        std::cout << "unitTests " << e.what() << std::endl;
        return 1;
    }
}

int main(int argc, char** argv) {
    const char* address;
    if(argc == 2){
        address = argv[1];
    } else {
        address = "127.0.0.1";
    }
    std::cout << "Server address : "  << address << std::endl;
    signal(SIGABRT , handler);   // install our handler
    signal(SIGSEGV, handler);   // install our handler

//    testSpeed(address);
    return unitTests(address);
}


