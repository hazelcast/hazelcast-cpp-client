/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "TestInvalidReadPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInvalidReadPortable::TestInvalidReadPortable() {

            }

            TestInvalidReadPortable::TestInvalidReadPortable(long l, int i, std::string s) {
                this->l = l;
                this->i = i;
                this->s = s;
            }

            int TestInvalidReadPortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestInvalidReadPortable::getClassId() const {
                return TestSerializationConstants::TEST_INVALID_READ_PORTABLE;
            }

            void TestInvalidReadPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("l", l);
                writer.writeInt("i", i);
                writer.writeUTF("s", &s);
            }


            void TestInvalidReadPortable::readPortable(serialization::PortableReader& reader) {
                l = reader.readLong("l");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                i = in.readInt();
                s = *reader.readUTF("s");
            }
        }
    }
}
