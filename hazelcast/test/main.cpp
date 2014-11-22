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

using namespace hazelcast::client::test;

void testSpeed() {
    SimpleMapTest s(HOST, 5701);
    s.run();
}

int unitTests() {
    try {
        RUN_TEST(ClientUtilTest, 1);
        RUN_TEST(ClientSerializationTest, 1);
        HazelcastServerFactory factory;
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

int main() {
//    testSpeed();
    return unitTests();
}


