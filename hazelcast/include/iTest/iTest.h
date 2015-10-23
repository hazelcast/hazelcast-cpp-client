/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/1/13.


#ifndef HAZELCAST_iTest
#define HAZELCAST_iTest

#include "iTest/iTestFixture.h"
#include <cstdlib>

namespace iTest {
    HAZELCAST_API void assertFalse(bool isFalse, const char *message);

    HAZELCAST_API void assertFalse(bool isFalse);

    HAZELCAST_API void assertTrue(bool isTrue, const char *message);

    HAZELCAST_API void assertTrue(bool isTrue);

    HAZELCAST_API void assertNull(const void *expected);

    HAZELCAST_API void assertNull(const void *expected, const char *message);
    
    HAZELCAST_API void assertNotNull(const void *expected);
    
    HAZELCAST_API void assertNotNull(const void *expected, const char *message);

#define ASSERT_EQUAL(expected, actual) \
    if (expected != actual) { \
    iTest::iTestException e; \
    std::stringstream s; \
    s << "ASSERT_EQUAL(" #expected ", " #actual")" << " failed at line " << __LINE__ << " in file " << __FILE__<< std::endl; \
    s << "Expected value:" << expected << ", Actual value:" << actual << std::endl; \
    e.message = s.str(); \
    throw e; \
    }

#define ASSERT_NOTEQUAL(expected, actual) \
    if (expected == actual) { \
    iTest::iTestException e; \
    std::stringstream s; \
    s << "ASSERT_EQUAL(" #expected ", " #actual")" << " failed at line " << __LINE__ << " in file " << __FILE__<< std::endl; \
    s << "Value should NOT be equal to :" << actual << std::endl; \
    e.message = s.str(); \
    throw e; \
    }

    template<typename Expected, typename Actual>
    void assertEqual(const Expected& expected, const Actual& actual, const char *message) {
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
    void assertEqual(const Expected& expected, const Actual& actual) {
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
    void assertEqualWithEpsilon(const Type& value1, const Type& value2, const Type& epsilon) {
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

