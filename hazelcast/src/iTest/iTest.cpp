//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "iTest/iTest.h"

namespace iTest {
    void assertFalse(bool isFalse, const char *message) {
        assertNumber++;
        if (isFalse) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed. Message : " << message << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    void assertFalse(bool isFalse) {
        assertNumber++;
        if (isFalse) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed." << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    void assertTrue(bool isTrue, const char *message) {
        assertNumber++;
        if (!isTrue) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed. Message : " << message << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    void assertTrue(bool isTrue) {
        assertNumber++;
        if (!isTrue) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed." << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    void assertNull(const void *expected) {
        assertNumber++;
        if (expected != NULL) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed." << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    void assertNotNull(const void *expected) {
        assertNumber++;
        if (expected == NULL) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed." << std::endl;
            e.message = s.str();
            throw e;
        }
    }
}

