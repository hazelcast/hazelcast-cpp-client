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

    void assertNull(const void *expected, const char *message) {
        assertNumber++;
        if (expected != NULL) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed. Message : " << message << std::endl;
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

    void assertNotNull(const void *expected, const char *message) {
        assertNumber++;
        if (expected == NULL) {
            iTestException e;
            std::stringstream s;
            s << "Assert[ "<< assertNumber << " ] is failed. Message : " << message  << std::endl;
            e.message = s.str();
            throw e;
        }
    }
}

