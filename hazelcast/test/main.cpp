#include "SimpleMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "ClientSerializationTest.h"
#include "ClientMapTest.h"
#include "ClientQueueTest.h"
#include "testUtil.h"

using namespace hazelcast::client::test;

int testSpeed() {
    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
    s.run();
    return 0;
};


int main(int argc, char **argv) {
    ClientSerializationTest serializationTest;
    serializationTest.executeTests();
    HazelcastInstanceFactory factory;
    ClientMapTest mapTest(factory);
    mapTest.executeTests();
    ClientQueueTest queueTest(factory);
    queueTest.executeTests();
    //IAtomicLongTest atomTest;
    //atomTest.executeTests();
//    IdGeneratorTest generatorTest;
//    generatorTest.executeTests();
//    testSpeed();
    return 0;
};

