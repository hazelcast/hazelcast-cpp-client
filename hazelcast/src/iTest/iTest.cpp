//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "iTest.h"

namespace iTest {
    void assertFalse(bool isFalse, const char *message) {
        if (isFalse) {
            std::cout << ">> Assert failed. message : " << message << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    };

    void assertFalse(bool isFalse) {
        if (isFalse) {
            std::cout << ">> Assert failed" << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    };

    void assertTrue(bool isTrue, const char *message) {
        if (!isTrue) {
            std::cout << ">> Assert failed. message : " << message << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    };

    void assertTrue(bool isTrue) {
        if (!isTrue) {
            std::cout << ">> Assert failed" << std::endl;
        } else {
            std::cout << "OK" << std::endl;
        }
    };
}