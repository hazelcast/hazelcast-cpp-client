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
#include <cstdlib>

using namespace hazelcast::client::test;

void testSpeed() {
    SimpleMapTest s(HOST, 5701);
    s.run();
}

void unitTests() {
    try {

        HazelcastServerFactory factory;

        MemberAttributeTest memberAttributeTest(factory);
        memberAttributeTest.executeTests();

        ClusterTest clusterTest(factory);
        clusterTest.executeTests();

        ClientSerializationTest serializationTest;
        serializationTest.executeTests();

        ClientMapTest mapTest(factory);
        mapTest.executeTests();

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
    } catch(std::exception &e) {
        std::cout << "unitTests " << e.what() << std::endl;
    }
}

void s(hazelcast::util::ThreadArgs &args) {
    int *v = (int *) args.arg0;
    std::cout << "Sleep" << std::endl;
	hazelcast::util::sleep(5);
    std::cout << "DONE " << *v << std::endl;
}

void testSleep() {
    int a = 5;
    hazelcast::util::Thread d(s, &a);
    time_t t = time(NULL);
    d.interrupt();
    d.join();
    double diff = difftime(time(NULL), t);
    std::cout << "Last " << diff << " seconcs" << std::endl;
}


void testLatchThreadMain(hazelcast::util::ThreadArgs &args) {
    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
    std::cout << "SLEEP" << std::endl;
	hazelcast::util::sleep(10);
    std::cout << "WAKEUP" << std::endl;
    latch->countDown();
}

void testLatch() {
    hazelcast::util::CountDownLatch latch(1);
    hazelcast::util::Thread thread(testLatchThreadMain, &latch);

    std::cout << "START AWAIT" << std::endl;
    bool b = latch.await(30);
    std::cout << b << std::endl;
    thread.join();

}

void testJoinThread(hazelcast::util::ThreadArgs &args) {
    long i = 0;
    try{
        while (i < 1000000000000L) {
            ++i;
            //std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" <<i++ << std::endl;
            //sleep(1);
        }
        hazelcast::client::exception::IException a("1", "2");
        throw a;
    }catch(hazelcast::client::exception::IException &e){
        std::cout << "skdsjdsjkjdksj" << e.what() << std::endl;
    }
}

void testJoin() {
    hazelcast::util::Thread *thread = new hazelcast::util::Thread(testJoinThread);
    //thread.interrupt();
    thread->join();
    delete thread;
    std::cout << "Joined" << std::endl;
}

int main() {
//    testJoin();
//    testLatch();
//    testSleep();
    time_t a = time(NULL);
    sleep(10);
    double s = difftime(a, time(NULL));
    std::cout << s << std::endl;
//    unitTests();
//    testSpeed();
    return 0;
}


