/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 5/6/13.

#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include "TestNamedPortable.h"
#include "TestDataSerializable.h"
#include "hazelcast/client/serialization/PortableFactory.h"
#include "TestSerializationConstants.h"
#include <vector>

#include <stdint.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestRawDataPortable : public serialization::Portable {
            public:
                static const int32_t CLASS_ID = TestSerializationConstants::TEST_RAW_DATA_PORTABLE;

                TestRawDataPortable(int64_t l, std::vector<char> c, TestNamedPortable p, int32_t k, std::string s, TestDataSerializable ds);

                TestRawDataPortable();

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                bool operator ==(const TestRawDataPortable &m) const;

                bool operator !=(const TestRawDataPortable &m) const;

                int64_t l;
                std::vector<char> c;
                TestNamedPortable p;
                int32_t k;
                std::string s;
                TestDataSerializable ds;
            };

            class TestDataPortableFactory : public serialization::PortableFactory {
            public:
                virtual std::unique_ptr<serialization::Portable> create(int32_t classId) const;
            };
        }
    }
}


#endif //__RawDataPortable_H_


