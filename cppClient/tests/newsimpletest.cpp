/* 
 * File:   newsimpletest.cpp
 * Author: msk
 *
 * Created on Feb 4, 2013, 2:33:58 PM
 */

#include <stdlib.h>
#include <iostream>

/*
 * Simple C++ Test Suite
 */

void hazelcast::client::IMap::remove(K key);

void testRemove() {
    K key;
    hazelcast::client::IMap iMap;
    iMap.remove(key);
    if (true /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRemove (newsimpletest) message=error message sample" << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% newsimpletest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRemove (newsimpletest)" << std::endl;
    testRemove();
    std::cout << "%TEST_FINISHED% time=0 testRemove (newsimpletest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

