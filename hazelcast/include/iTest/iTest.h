//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_iTest
#define HAZELCAST_iTest

#include "iTest/iTestFixture.h"
#include <string>
#include <iostream>

namespace iTest {
    void assertFalse(bool isFalse, const char *message);

    void assertFalse(bool isFalse);

    void assertTrue(bool isTrue, const char *message);

    void assertTrue(bool isTrue);

    void assertNull(const void *expected);

    void assertNotNull(const void *expected);

    template<typename  Expected, typename Actual >
    void assertEqual(const Expected &expected, const Actual &actual, const char *message) {
        if (expected != actual) {
            iTestException e;
            e.message.assign(">> Assert failed. message : ");
            e.message += message;
            throw e;
        }
    };

    template<typename  Expected, typename Actual >
    void assertEqual(const Expected &expected, const Actual &actual) {
        if (actual != expected) {
            iTestException e;
            e.message.assign(">> Assert failed");
            throw e;
        }
    };
}


#endif //HAZELCAST_iTest
