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

#include <gtest/gtest.h>
#include <TestHelperFunctions.h>
#include <ClientTestSupport.h>

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class PortableVersionTest : public ::testing::Test {
            protected:
                class Child : public serialization::Portable {
                public:
                    Child() {
                    }

                    Child(const std::string &name) : name (name) {
                    }

                    virtual int getFactoryId() const {
                        return 1;
                    }

                    virtual int getClassId() const {
                        return 2;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                        writer.writeUTF("name", &name);
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                        name = *reader.readUTF("name");
                    }

                    bool operator==(const Child &rhs) const {
                        return name == rhs.name;
                    }

                private:
                    std::string name;

                };

                class Parent : public serialization::Portable {
                public:
                    Parent() {}

                    Parent(const Child &child) : child(child) {}

                    virtual int getFactoryId() const {
                        return 1;
                    }

                    virtual int getClassId() const {
                        return 1;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                        writer.writePortable<Child>("child", &child);
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                        child = *reader.readPortable<Child>("child");
                    }

                    bool operator==(const Parent &rhs) const {
                        return child == rhs.child;
                    }

                    bool operator!=(const Parent &rhs) const {
                        return !(rhs == *this);
                    }

                private:
                    Child child;
                };

                class MyPortableFactory : public serialization::PortableFactory {
                public:
                    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
                        if (classId == 1) {
                            return std::auto_ptr<serialization::Portable>(new Parent());
                        } else if (classId == 2) {
                            return std::auto_ptr<serialization::Portable>(new Child());
                        }

                        return std::auto_ptr<serialization::Portable>();
                    }
                };
            };

            // Test for issue https://github.com/hazelcast/hazelcast/issues/12733
            TEST_F(PortableVersionTest, test_nestedPortable_versionedSerializer) {
                SerializationConfig serializationConfig;
                serializationConfig.addPortableFactory(1, boost::shared_ptr<serialization::PortableFactory>(new MyPortableFactory));
                serialization::pimpl::SerializationService ss1(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(6).addPortableFactory(1, boost::shared_ptr<serialization::PortableFactory>(new MyPortableFactory));
                serialization::pimpl::SerializationService ss2(serializationConfig2);

                //make sure ss2 cached class definition of Child
                ss2.toData<Child>(new Child("sancar"));

                //serialized parent from ss1
                Parent parent(Child("sancar"));
                serialization::pimpl::Data data = ss1.toData<Parent>(&parent);

                // cached class definition of Child and the class definition from data coming from ss1 should be compatible
                assertEquals(parent, *ss2.toObject<Parent>(data));

            }
        }
    }
}
