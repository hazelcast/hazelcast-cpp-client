//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ClientSerializationTest
#define HAZELCAST_ClientSerializationTest

#include "iTest/iTest.h"

namespace hazelcast {
    namespace client {
        namespace test {

            class ClientSerializationTest : public iTest::iTestFixture<ClientSerializationTest> {
            public:
                ClientSerializationTest(int dummy);

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void addTests();

                void testBasicFunctionality();

                void testBasicFunctionalityWithDifferentVersions();

                void testCustomSerialization();

                void testRawData();

                void testIdentifiedDataSerializable();

                void testRawDataWithoutRegistering();

                void testInvalidWrite();

                void testInvalidRead();

                void testDifferentVersions();

                void testTemplatedPortable_whenMultipleTypesAreUsed();

                void testDataHash();
            };
        }
    }
}


#endif //HAZELCAST_ClientSerializationTest

