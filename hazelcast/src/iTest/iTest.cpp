//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "iTest/iTest.h"

namespace iTest {
    void assertFalse(bool isFalse, const char *message) {
        if (isFalse) {
            iTestException e;
            e.message.assign(">> Assert failed. message : ");
            e.message += message;
            throw e;
        }
    };

    void assertFalse(bool isFalse) {
        if (isFalse) {
            iTestException e;
            e.message.assign(">> Assert failed");
            throw e;
        }
    };

    void assertTrue(bool isTrue, const char *message) {
        if (!isTrue) {
            iTestException e;
            e.message.assign(">> Assert failed. message : ");
            e.message += message;
            throw e;
        }
    };

    void assertTrue(bool isTrue) {
        if (!isTrue) {
            iTestException e;
            e.message.assign(">> Assert failed");
            throw e;
        }
    };

    void assertNull(const void *expected) {
        if (expected != NULL) {
            iTestException e;
            e.message.assign(">> Assert failed");
            throw e;
        }
    };

    void assertNotNull(const void *expected) {
        if (expected == NULL) {
            iTestException e;
            e.message.assign(">> Assert failed");
            throw e;
        }
    };
}