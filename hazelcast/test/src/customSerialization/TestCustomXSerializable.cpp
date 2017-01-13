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
#include "customSerialization/TestCustomXSerializable.h"

namespace hazelcast {
    namespace client {
        namespace test {


            TestCustomXSerializable::TestCustomXSerializable() {

            }

            TestCustomXSerializable::TestCustomXSerializable(int id) :id(id){

            }

            bool TestCustomXSerializable::operator==(const TestCustomXSerializable& rhs) const {
                if (this == &rhs)
                    return true;
                if (id != rhs.id) return false;
                return true;
            }

            bool TestCustomXSerializable::operator!=(const TestCustomXSerializable& rhs) const {
                return !(*this == rhs);
            }

            int getHazelcastTypeId(TestCustomXSerializable const* param) {
                return 666;
            }

            TestCustomPerson::TestCustomPerson() {

            }

            TestCustomPerson::TestCustomPerson(const std::string& name):name(name) {

            }

            bool TestCustomPerson::operator==(const TestCustomPerson& rhs) const {
                if (this == &rhs)
                    return true;
                if (name.compare(rhs.name))
                    return false;
                return true;
            }

            bool TestCustomPerson::operator!=(const TestCustomPerson& rhs) const {
                return !(*this == rhs);
            }

            void TestCustomPerson::setName(const std::string& name) {
                this->name = name;
            }

            std::string TestCustomPerson::getName() const {
                return name;
            }

            int getHazelcastTypeId(TestCustomPerson const* param) {
                return 999;
            }

        }
    }
}

