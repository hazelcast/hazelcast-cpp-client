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
#include "TestNamedPortableV2.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestNamedPortableV2::TestNamedPortableV2() {

            }

            TestNamedPortableV2::TestNamedPortableV2(std::string name, int v) : name(name), k(v * 10), v(v) {
            }

            int TestNamedPortableV2::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestNamedPortableV2::getClassId() const {
                return TestSerializationConstants::TEST_NAMED_PORTABLE_2;
            }


            void TestNamedPortableV2::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("v", v);
                writer.writeUTF("name", &name);
                writer.writeInt("myint", k);
            }


            void TestNamedPortableV2::readPortable(serialization::PortableReader& reader) {
                v = reader.readInt("v");
                name = *reader.readUTF("name");
                k = reader.readInt("myint");
            }

        }
    }
}
