#include "SimpleMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "serialization/ClientSerializationTest.h"
#include "map/ClientMapTest.h"
#include "queue/ClientQueueTest.h"
#include "serialization/testUtil.h"
#include "atomiclong/IAtomicLongTest.h"
#include "idgenerator/IdGeneratorTest.h"
#include "countdownlatch/ICountDownLatchTest.h"
#include "lock/ClientLockTest.h"
#include "semaphore/ClientSemaphoreTest.h"
#include "topic/ClientTopicTest.h"
#include "executor/IExecutorServiceTest.h"
#include "multimap/ClientMultiMapTest.h"
#include "list/ClientListTest.h"
#include "set/ClientSetTest.h"
#include "txn/ClientTxnListTest.h"
#include "txn/ClientTxnMapTest.h"
#include "txn/ClientTxnMultiMapTest.h"
#include "txn/ClientTxnQueueTest.h"
#include "txn/ClientTxnSetTest.h"
#include "txn/ClientTxnTest.h"
#include "map/ClientMapIssueTest.h"

using namespace hazelcast::client::test;

int testSpeed() {
    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
    s.run();
    return 0;
};

int main(int argc, char **argv) {
    HazelcastInstanceFactory factory;
    ClientSerializationTest serializationTest;
    serializationTest.executeTests();
    ClientMapTest mapTest(factory);
    mapTest.executeTests();
    ClientMapIssueTest mapIssueTest(factory);
    mapIssueTest.executeTests();
    ClientMultiMapTest multiMapTest(factory);
    multiMapTest.executeTests();
    ClientQueueTest queueTest(factory);
    queueTest.executeTests();
    ClientListTest listTest(factory);
    listTest.executeTests();
    ClientSetTest setTest(factory);
    setTest.executeTests();
    IAtomicLongTest atomTest(factory);
    atomTest.executeTests();
    IdGeneratorTest generatorTest(factory);
    generatorTest.executeTests();
    ICountDownLatchTest latchTest(factory);
    latchTest.executeTests();
    ClientLockTest lockTest(factory);
    lockTest.executeTests();
    ClientSemaphoreTest semaphoreTest(factory);
    semaphoreTest.executeTests();
    ClientTopicTest topicTest(factory);
    topicTest.executeTests();
    IExecutorServiceTest executorServiceTest(factory);
    executorServiceTest.executeTests();
    ClientTxnListTest clientTxnListTest(factory);
    clientTxnListTest.executeTests();
    ClientTxnMapTest clientTxnMapTest(factory);
    clientTxnMapTest.executeTests();
    ClientTxnMultiMapTest clientTxnMultiMapTest(factory);
    clientTxnMultiMapTest.executeTests();
    ClientTxnQueueTest clientTxnQueueTest(factory);
    clientTxnQueueTest.executeTests();
    ClientTxnSetTest clientTxnSetTest(factory);
    clientTxnSetTest.executeTests();
    ClientTxnTest clientTxnTest(factory);
    clientTxnTest.executeTests();
//    testSpeed();
    return 0;
};

