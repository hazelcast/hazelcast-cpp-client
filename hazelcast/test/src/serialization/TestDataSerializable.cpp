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
#include "TestDataSerializable.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestDataSerializable::TestDataSerializable() {

            }

            TestDataSerializable::TestDataSerializable(int i, char c):i(i), c(c) {

            }

            bool TestDataSerializable::operator ==(const TestDataSerializable & rhs) const {
                if (this == &rhs)
                    return true;
                if (i != rhs.i) return false;
                if (c != rhs.c) return false;
                return true;
            }

            bool TestDataSerializable::operator !=(const TestDataSerializable& m) const {
                return !(*this == m);
            }

            int TestDataSerializable::getFactoryId() const {
                return TestSerializationConstants::TEST_DATA_FACTORY;
            }

            int TestDataSerializable::getClassId() const {
                return TestSerializationConstants::TEST_DATA_SERIALIZABLE;
            }

            void TestDataSerializable::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeChar(c);
                writer.writeInt(i);
            }

            void TestDataSerializable::readData(serialization::ObjectDataInput& reader) {
                c = reader.readChar();
                i = reader.readInt();
            }
        }
    }
}



