#ifndef MAPTEST
#define MAPTEST

#include "TestMainPortable.h"
#include "testUtil.h"
#include "SimpleMapTest.h"

int testSpeed() {
    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
    s.run();
    return 0;
};

#endif

