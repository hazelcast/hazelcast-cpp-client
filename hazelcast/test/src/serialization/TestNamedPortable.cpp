/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "TestNamedPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {

            TestNamedPortable::TestNamedPortable() {
            }

            TestNamedPortable::TestNamedPortable(std::string name, int k):name(name), k(k) {
            }

            int TestNamedPortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestNamedPortable::getClassId() const {
                return TestSerializationConstants::TEST_NAMED_PORTABLE;
            }

            void TestNamedPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", &name);
                writer.writeInt("myint", k);
            }


            void TestNamedPortable::readPortable(serialization::PortableReader& reader) {
                name = *reader.readUTF("name");
                k =  reader.readInt("myint");
            }

            bool TestNamedPortable::operator ==(const TestNamedPortable& m) const {
                if (this == &m)
                    return true;
                if (k != m.k)
                    return false;
                if (name.compare(m.name))
                    return false;
                return true;
            }

            bool TestNamedPortable::operator !=(const TestNamedPortable& m) const {
                return !(*this == m);
            }

        }
    }
}
