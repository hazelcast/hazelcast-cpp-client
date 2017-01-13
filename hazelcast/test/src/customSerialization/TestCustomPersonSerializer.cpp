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
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "customSerialization/TestCustomXSerializable.h"
#include "customSerialization/TestCustomPersonSerializer.h"

namespace hazelcast {
    namespace client {
        namespace test {

            void TestCustomPersonSerializer::write(serialization::ObjectDataOutput & out, const TestCustomPerson& object) {
                out.writeInt(999);
                const std::string &name = object.getName();
                out.writeUTF(&name);
                out.writeInt(999);
            }

            void TestCustomPersonSerializer::read(serialization::ObjectDataInput & in, TestCustomPerson& object) {
                int i = in.readInt();
                assert(i == 999);
                object.setName(*in.readUTF());
                i = in.readInt();
                assert(i == 999);
            }

            int TestCustomPersonSerializer::getHazelcastTypeId() const {
                return 999;
            }

        }
    }
}
