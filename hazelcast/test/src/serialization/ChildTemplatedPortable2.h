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
//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_ChildTemplatedPortable2
#define HAZELCAST_ChildTemplatedPortable2


#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {

            class ChildTemplatedPortable2 : public serialization::Portable {
            public:
                ChildTemplatedPortable2(){

                }

                ChildTemplatedPortable2(std::string s1):s1(s1){

                }

                int getFactoryId() const {
                    return TestSerializationConstants::TEST_DATA_FACTORY;
                }

                int getClassId() const {
                    return TestSerializationConstants::CHILD_TEMPLATED_PORTABLE_2;
                }

                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("s", &s1);
                }

                void readPortable(serialization::PortableReader& reader) {
                    s1 = *reader.readUTF("s");
                }

                std::string s1;
            private:
            };
        }
    }
}

#endif //HAZELCAST_ChildTemplatedPortable2
