//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ClientSerializationTest
#define HAZELCAST_ClientSerializationTest

#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {
        namespace test {

            class ClientSerializationTest : public ClientTestSupport<ClientSerializationTest> {
            public:
                ClientSerializationTest();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void addTests();

                void testBasicFunctionality();

                void testBasicFunctionalityWithLargeData();

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

                void testPrimitives();

                void testPrimitiveArrays();

                void testWriteObjectWithPortable();

                void testWriteObjectWithIdentifiedDataSerializable();

                void testWriteObjectWithCustomXSerializable();

                void testWriteObjectWithCustomPersonSerializable();

                void testNullData();

                void testMorphingWithDifferentTypes_differentVersions();

                template<typename T>
                T toDataAndBackToObject(serialization::pimpl::SerializationService& ss, T& value) {
                    serialization::pimpl::Data data = ss.toData<T>(&value);
                    return *(ss.toObject<T>(data));
                }
            };
        }
    }
}


#endif //HAZELCAST_ClientSerializationTest

