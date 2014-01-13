#include "SimpleMapTest.h"
#include "map/ClientMapTest.h"
#include "queue/ClientQueueTest.h"
#include "serialization/testUtil.h"
#include "atomiclong/IAtomicLongTest.h"
#include "idgenerator/IdGeneratorTest.h"
#include "countdownlatch/ICountDownLatchTest.h"
#include "lock/ClientLockTest.h"
#include "semaphore/ClientSemaphoreTest.h"
#include "topic/ClientTopicTest.h"
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
#include "ByteBufferTest.h"

using namespace hazelcast::client::test;

int testSpeed() {
    SimpleMapTest s("192.168.2.201", 5701);
    s.run();
    return 0;
};

int main(int argc, char **argv) {

//    Address address("192.168.2.201", 5701);
//    ClientConfig clientConfig;
//    clientConfig.addAddress(address);
//    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
//    HazelcastClient hazelcastClient(clientConfig);
//    IMap<int, std::string> iMap = hazelcastClient.getMap<int,std::string>("deneme");
//    (std::cout << iMap.getName() << std::endl);
//    iMap.put(1, "sss");
//    boost::shared_ptr<string> ptr = iMap.get(1);
//    if(ptr != NULL){
//        (std::cout << *ptr  << std::endl);
//    }

//    HazelcastInstanceFactory factory;

    //ClientSerializationTest serializationTest;
    //serializationTest.executeTests();

//    ClientMapTest mapTest(factory);
//    mapTest.executeTests();
//
//    ClientMapIssueTest mapIssueTest(factory);
//    mapIssueTest.executeTests();
//
//    ClientMultiMapTest multiMapTest(factory);
//    multiMapTest.executeTests();
//
//    ClientQueueTest queueTest(factory);
//    queueTest.executeTests();
//
//    ClientListTest listTest(factory);
//    listTest.executeTests();
//
//    ClientSetTest setTest(factory);
//    setTest.executeTests();
//
//    IAtomicLongTest atomTest(factory);
//    atomTest.executeTests();
//
//    IdGeneratorTest generatorTest(factory);
//    generatorTest.executeTests();
//
//    ICountDownLatchTest latchTest(factory);
//    latchTest.executeTests();
//
//    ClientLockTest lockTest(factory);
//    lockTest.executeTests();
//
//    ClientSemaphoreTest semaphoreTest(factory);
//    semaphoreTest.executeTests();
//
//    ClientTopicTest topicTest(factory);
//    topicTest.executeTests();
//
//    ClientTxnListTest clientTxnListTest(factory);
//    clientTxnListTest.executeTests();
//
//    ClientTxnMapTest clientTxnMapTest(factory);
//    clientTxnMapTest.executeTests();
//
//    ClientTxnMultiMapTest clientTxnMultiMapTest(factory);
//    clientTxnMultiMapTest.executeTests();
//
//    ClientTxnQueueTest clientTxnQueueTest(factory);
//    clientTxnQueueTest.executeTests();
//
//    ClientTxnSetTest clientTxnSetTest(factory);
//    clientTxnSetTest.executeTests();
//
//    ClientTxnTest clientTxnTest(factory);
//    clientTxnTest.executeTests();

    testSpeed();


    return 0;
};

