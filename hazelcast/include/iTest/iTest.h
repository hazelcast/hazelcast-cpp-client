//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_iTest
#define HAZELCAST_iTest

#include "iTest/iTestFixture.h"


namespace iTest {
    HAZELCAST_API void assertFalse(bool isFalse, const char *message);

    HAZELCAST_API void assertFalse(bool isFalse);

    HAZELCAST_API void assertTrue(bool isTrue, const char *message);

    HAZELCAST_API void assertTrue(bool isTrue);

    HAZELCAST_API void assertNull(const void *expected);

    HAZELCAST_API void assertNotNull(const void *expected);

    template<typename Expected, typename Actual>
    HAZELCAST_API void assertEqual(const Expected& expected, const Actual& actual, const char *message) {
        assertNumber++;
        if (expected != actual) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ " << assertNumber << " ] is failed. Message : " << message << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    template<typename Expected, typename Actual>
    HAZELCAST_API void assertEqual(const Expected& expected, const Actual& actual) {
        assertNumber++;
        if (actual != expected) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ " << assertNumber << " ] is failed." << std::endl;
            e.message = s.str();
            throw e;
        }
    }

    template<typename Type>
    HAZELCAST_API void assertEqualWithEpsilon(const Type& value1, const Type& value2, const Type& epsilon) {
        assertNumber++;
        if (std::abs(value1 - value2) > epsilon) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ " << assertNumber << " ] is failed." << std::endl;
            s << "Message: " << value1 << " != " << value2 << " : epsilon " << epsilon << std::endl;
            e.message = s.str();
            throw e;
        }
    }
}


#endif //HAZELCAST_iTest

