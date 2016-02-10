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
// Created by sancar koyunlu on 05/04/15.
//

#include "TestNamedPortableV3.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {

            TestNamedPortableV3::TestNamedPortableV3() {
            }

            TestNamedPortableV3::TestNamedPortableV3(std::string name, short k):name(name), k(k) {
            }

            int TestNamedPortableV3::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestNamedPortableV3::getClassId() const {
                return TestSerializationConstants::TEST_NAMED_PORTABLE_3;
            }

            void TestNamedPortableV3::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", &name);
                writer.writeShort("myint", k);
            }


            void TestNamedPortableV3::readPortable(serialization::PortableReader& reader) {
                name = *reader.readUTF("name");
                k = reader.readShort("myint");
            }

            bool TestNamedPortableV3::operator ==(const TestNamedPortableV3& m) const {
                if (this == &m)
                    return true;
                if (k != m.k)
                    return false;
                if (name.compare(m.name))
                    return false;
                return true;
            }

            bool TestNamedPortableV3::operator !=(const TestNamedPortableV3& m) const {
                return !(*this == m);
            }

        }
    }
}
