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
#include "TestInvalidWritePortable.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInvalidWritePortable::TestInvalidWritePortable() {

            }

            TestInvalidWritePortable::TestInvalidWritePortable(long l, int i, std::string s) {
                this->l = l;
                this->i = i;
                this->s = s;
            }

            int TestInvalidWritePortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestInvalidWritePortable::getClassId() const {
                return TestSerializationConstants::TEST_INVALID_WRITE_PORTABLE;
            }

            void TestInvalidWritePortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("l", l);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeInt(i);
                writer.writeUTF("s", &s);
            }

            void TestInvalidWritePortable::readPortable(serialization::PortableReader& reader) {
                l = reader.readLong("l");
                i = reader.readInt("i");
                s = *reader.readUTF("s");
            }
        }
    }
}
