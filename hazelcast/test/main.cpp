#include "SimpleMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "ClientSerializationTest.h"
#include "ClientMapTest.h"
#include "ClientQueueTest.h"
#include "testUtil.h"
#include "IAtomicLongTest.h"
#include "IdGeneratorTest.h"
#include "ICountDownLatchTest.h"
#include "ClientLockTest.h"

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
    ClientQueueTest queueTest(factory);
    queueTest.executeTests();
    IAtomicLongTest atomTest(factory);
    atomTest.executeTests();
    IdGeneratorTest generatorTest(factory);
    generatorTest.executeTests();
    ICountDownLatchTest latchTest(factory);
    latchTest.executeTests();
    ClientLockTest lockTest(factory);
    lockTest.executeTests();
//    testSpeed();
    return 0;
};

