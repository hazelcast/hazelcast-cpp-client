//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_iTestFixtureSample
#define HAZELCAST_iTestFixtureSample

#include "iTest.h"

using namespace iTest;

class iTestFixtureSample : public iTestFixture<iTestFixtureSample> {
public:
    iTestFixtureSample() {
    };

    void addTests() {
        addTest(&iTestFixtureSample::testA, "testA");
        addTest(&iTestFixtureSample::testB, "testB");
    };

    void beforeClass() {
        std::cout << "beforeClass" << std::endl;
    };

    void afterClass() {
        std::cout << "afterClass" << std::endl;
    };

    void beforeTest() {
        std::cout << "beforeTest" << std::endl;
    };

    void afterTest() {
        std::cout << "afterTest" << std::endl;
    };

    void testA() {
        std::cout << "testA" << std::endl;
        assertEqual(2, 1);
    };

    void testB() {
        std::cout << "testB" << std::endl;
        assertEqual(1, 1);
    };
};


#endif //HAZELCAST_iTestFixtureSample