//#include "deneme.h"
#include "SimpleMapTest.h"
#include "serializationTest.h"
#include "QueueTest.h"
#include "mapTest.h"

int testSpeed() {
    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
    s.run();
    return 0;
};

int main(int argc, char **argv) {
    QueueTest queueTest;
    queueTest.executeTests();
    SerializationTest serializationTest;
    serializationTest.executeTests();
    MapTest mapTest;
    mapTest.executeTests();
    return 0;

//    return testSpeed();
//    return deneme::init();
};

