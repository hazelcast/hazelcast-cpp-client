//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_iTest
#define HAZELCAST_iTest

#include "iTestFixture.h"
#include <string>
#include <iostream>

namespace iTest {
    void assertFalse(bool isFalse, const char *message);

    void assertFalse(bool isFalse);

    void assertTrue(bool isTrue, const char *message);

    void assertTrue(bool isTrue);

    template<typename  Exptected, typename Actual >
    void assertEqual(const Exptected& expected, const Actual& actual, const char *message) {
        if (expected != actual) {
            std::cout << ">> Assert failed. message : " << message << std::endl;

        } else {
            std::cout << "OK" << std::endl;
        }
    };

    template<typename  Exptected, typename Actual >
    void assertEqual(const Exptected& expected, const Actual& actual) {
        if (expected != actual) {
            std::cout << ">> Assert failed." << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    };
}


#endif //HAZELCAST_iTest
