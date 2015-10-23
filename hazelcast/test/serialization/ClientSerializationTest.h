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
// Created by sancar koyunlu on 8/27/13.


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

