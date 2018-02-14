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

#include <ClientTestSupport.h>
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareTest : public ClientTestSupport {
            protected:
                class SimplePartitionAwareObject
                        : public PartitionAware<int>, public serialization::IdentifiedDataSerializable {
                public:
                    SimplePartitionAwareObject() : testKey(5) {}

                    virtual const int *getPartitionKey() const {
                        return &testKey;
                    }

                    int getFactoryId() const {
                        return 1;
                    }

                    int getClassId() const {
                        return 2;
                    }

                    void writeData(serialization::ObjectDataOutput &writer) const {
                    }

                    void readData(serialization::ObjectDataInput &reader) {
                    }

                    int getTestKey() const {
                        return testKey;
                    }
                private:
                    int testKey;
                };
            };

            TEST_F(PartitionAwareTest, testSimplePartitionAwareObjectSerialisation) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SimplePartitionAwareObject obj;
                serialization::pimpl::Data data = serializationService.toData<SimplePartitionAwareObject>(&obj);
                ASSERT_TRUE(data.hasPartitionHash());

                int testKey = obj.getTestKey();
                serialization::pimpl::Data expectedData = serializationService.toData<int>(&testKey);

                ASSERT_EQ(expectedData.getPartitionHash(), data.getPartitionHash());
            }

            TEST_F(PartitionAwareTest, testNonPartitionAwareObjectSerialisation) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                int obj = 7;
                serialization::pimpl::Data data = serializationService.toData<int>(&obj);
                ASSERT_FALSE(data.hasPartitionHash());
            }
        }
    }
}



