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
// Created by sancar koyunlu on 9/13/13.

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/HazelcastClient.h"

#include "../HazelcastServerFactory.h"
#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class PolymorphicDataSerializableRingBufferTest : public ClientTestSupport {
            protected:
                class BaseDataSerializable : public serialization::IdentifiedDataSerializable {
                public:
                    virtual ~BaseDataSerializable() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 10;
                    }

                    virtual void writeData(serialization::ObjectDataOutput &writer) const {
                    }

                    virtual void readData(serialization::ObjectDataInput &reader) {
                    }

                    virtual bool operator<(const BaseDataSerializable &rhs) const {
                        return getClassId() < rhs.getClassId();
                    }
                };

                class Derived1DataSerializable : public BaseDataSerializable {
                public:
                    virtual int getClassId() const {
                        return 11;
                    }
                };

                class Derived2DataSerializable : public Derived1DataSerializable {
                public:
                    virtual int getClassId() const {
                        return 12;
                    }
                };

                class PolymorphicDataSerializableFactory : public serialization::DataSerializableFactory {
                public:
                    virtual std::auto_ptr<serialization::IdentifiedDataSerializable> create(int32_t typeId) {
                        switch (typeId) {
                            case 10:
                                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new BaseDataSerializable);
                            case 11:
                                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Derived1DataSerializable);
                            case 12:
                                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Derived2DataSerializable);
                            default:
                                return std::auto_ptr<serialization::IdentifiedDataSerializable>();
                        }
                    }
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig;
                    SerializationConfig &serializationConfig = clientConfig->getSerializationConfig();
                    serializationConfig.addDataSerializableFactory(666,
                                                                   boost::shared_ptr<serialization::DataSerializableFactory>(
                                                                           new PolymorphicDataSerializableFactory()));
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    rb = client->getRingbuffer<BaseDataSerializable>("rb-1");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete clientConfig;
                    delete instance;

                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static boost::shared_ptr<Ringbuffer<BaseDataSerializable> > rb;

                static const int64_t CAPACITY;
            };

            const int64_t PolymorphicDataSerializableRingBufferTest::CAPACITY = 10;

            HazelcastServer *PolymorphicDataSerializableRingBufferTest::instance = NULL;
            ClientConfig *PolymorphicDataSerializableRingBufferTest::clientConfig = NULL;
            HazelcastClient *PolymorphicDataSerializableRingBufferTest::client = NULL;
            boost::shared_ptr<Ringbuffer<PolymorphicDataSerializableRingBufferTest::BaseDataSerializable> > PolymorphicDataSerializableRingBufferTest::rb;

            TEST_F(PolymorphicDataSerializableRingBufferTest, testPolymorhism) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                rb->add(base);
                rb->add(derived1);
                rb->add(derived2);

                int64_t sequence = rb->headSequence();
                std::auto_ptr<BaseDataSerializable> value = rb->readOne(sequence);
                ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                ASSERT_EQ(base.getClassId(), value->getClassId());

                value = rb->readOne(sequence + 1);
                ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                ASSERT_EQ(derived1.getClassId(), value->getClassId());

                value = rb->readOne(sequence + 2);
                ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                ASSERT_EQ(derived2.getClassId(), value->getClassId());
            }
        }
    }
}

