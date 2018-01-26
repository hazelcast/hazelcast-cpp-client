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

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <memory>
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/adaptor/RawPointerMap.h"

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class BaseCustom {
            public:
                BaseCustom() {
                    value = 3;
                }

                BaseCustom(int value) : value(value) {}

                int getValue() const {
                    return value;
                }

                void setValue(int value) {
                    BaseCustom::value = value;
                }

                bool operator<(const BaseCustom &rhs) const {
                    return getValue() < rhs.getValue();
                }

            private:
                int value;
            };

            class Derived1Custom : public BaseCustom {
            public:
                Derived1Custom() : BaseCustom(4) {}

                Derived1Custom(int value) : BaseCustom(value) {}
            };

            class Derived2Custom : public Derived1Custom {
            public:
                Derived2Custom() : Derived1Custom(5) {}
            };

            int32_t getHazelcastTypeId(const BaseCustom *) {
                return 3;
            }

            int32_t getHazelcastTypeId(const Derived1Custom *) {
                return 4;
            }

            int32_t getHazelcastTypeId(const Derived2Custom *) {
                return 5;
            }

            class MixedMapTest : public ClientTestSupport {
            protected:
                class BaseCustomSerializer : public serialization::StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 3;
                    }

                    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
                        out.writeInt(static_cast<const BaseCustom *>(object)->getValue());
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::auto_ptr<BaseCustom> object(new BaseCustom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

                class Derived1CustomSerializer : public BaseCustomSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 4;
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::auto_ptr<Derived1Custom> object(new Derived1Custom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

                class Derived2CustomSerializer : public BaseCustomSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 5;
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::auto_ptr<Derived2Custom> object(new Derived2Custom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

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

                class BasePortable : public serialization::Portable {
                public:
                    virtual ~BasePortable() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 3;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                    }

                    bool operator<(const BasePortable &rhs) const {
                        return getClassId() < rhs.getClassId();
                    }
                };

                class Derived1Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 4;
                    }
                };

                class Derived2Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 5;
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
                
                class PolymorphicPortableFactory : public serialization::PortableFactory {
                public:
                    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
                        switch (classId) {
                            case 3:
                                return std::auto_ptr<serialization::Portable>(new BasePortable);
                            case 4:
                                return std::auto_ptr<serialization::Portable>(new Derived1Portable);
                            case 5:
                                return std::auto_ptr<serialization::Portable>(new Derived2Portable);
                            default:
                                return std::auto_ptr<serialization::Portable>();
                        }
                    }
                };

                virtual void TearDown() {
                    // clear maps
                    try {
                        mixedMap->clear();
                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "[TearDown] An exception occured in tear down:" << e.what();
                        util::ILogger::getLogger().warning(out.str());
                    }
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    mixedMap = new mixedtype::IMap(client->toMixedType().getMap("MyMap"));

                    ClientConfig config;
                    SerializationConfig &serializationConfig = config.getSerializationConfig();
                    serializationConfig.addDataSerializableFactory(666,
                                                                   boost::shared_ptr<serialization::DataSerializableFactory>(
                                                                                       new PolymorphicDataSerializableFactory()));
                    serializationConfig.addPortableFactory(666, boost::shared_ptr<serialization::PortableFactory>(
                            new PolymorphicPortableFactory));

                    serializationConfig.registerSerializer(
                            boost::shared_ptr<serialization::SerializerBase>(new BaseCustomSerializer));

                    serializationConfig.registerSerializer(
                            boost::shared_ptr<serialization::SerializerBase>(new Derived1CustomSerializer));

                    serializationConfig.registerSerializer(
                            boost::shared_ptr<serialization::SerializerBase>(new Derived2CustomSerializer));

                    client2 = new HazelcastClient(config);
                    imap = new IMap<int, BaseDataSerializable>(client2->getMap<int, BaseDataSerializable>("MyMap"));
                    rawPointerMap = new adaptor::RawPointerMap<int, BaseDataSerializable> (*imap);
                    imapPortable = new IMap<int, BasePortable>(client2->getMap<int, BasePortable>("MyMap"));
                    rawPointerMapPortable = new adaptor::RawPointerMap<int, BasePortable> (*imapPortable);
                    imapCustom = new IMap<int, BaseCustom>(client2->getMap<int, BaseCustom>("MyMap"));
                    rawPointerMapCustom = new adaptor::RawPointerMap<int, BaseCustom> (*imapCustom);
                }

                static void TearDownTestCase() {
                    //delete mixedMap;
                    delete rawPointerMap;
                    delete imap;
                    delete rawPointerMapPortable;
                    delete imapPortable;
                    delete rawPointerMapCustom;
                    delete imapCustom;
                    delete client;
                    delete client2;
                    delete clientConfig;
                    delete instance2;
                    delete instance;

                    mixedMap = NULL;
                    rawPointerMap = NULL;
                    imap = NULL;
                    rawPointerMapPortable = NULL;
                    imapPortable = NULL;
                    rawPointerMapCustom = NULL;
                    imapCustom = NULL;
                    client = NULL;
                    client2 = NULL;
                    clientConfig = NULL;
                    instance2 = NULL;
                    instance = NULL;
                }
                
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static mixedtype::IMap *mixedMap;
                static HazelcastClient *client2;
                static IMap<int, BaseDataSerializable> *imap;
                static adaptor::RawPointerMap<int, BaseDataSerializable> *rawPointerMap;
                static IMap<int, BasePortable> *imapPortable;
                static adaptor::RawPointerMap<int, BasePortable> *rawPointerMapPortable;
                static IMap<int, BaseCustom> *imapCustom;
                static adaptor::RawPointerMap<int, BaseCustom> *rawPointerMapCustom;
            };

            HazelcastServer *MixedMapTest::instance = NULL;
            HazelcastServer *MixedMapTest::instance2 = NULL;
            ClientConfig *MixedMapTest::clientConfig = NULL;
            HazelcastClient *MixedMapTest::client = NULL;
            mixedtype::IMap *MixedMapTest::mixedMap = NULL;
            HazelcastClient *MixedMapTest::client2 = NULL;
            IMap<int, MixedMapTest::BaseDataSerializable> * MixedMapTest::imap = NULL;
            adaptor::RawPointerMap<int, MixedMapTest::BaseDataSerializable> *MixedMapTest::rawPointerMap = NULL;
            IMap<int, MixedMapTest::BasePortable> * MixedMapTest::imapPortable = NULL;
            adaptor::RawPointerMap<int, MixedMapTest::BasePortable> *MixedMapTest::rawPointerMapPortable = NULL;
            IMap<int, BaseCustom> * MixedMapTest::imapCustom = NULL;
            adaptor::RawPointerMap<int, BaseCustom> *MixedMapTest::rawPointerMapCustom = NULL;

            TEST_F(MixedMapTest, testPutDifferentTypes) {
                mixedMap->put<int, int>(3, 5);
                TypedData oldData = mixedMap->put<int, std::string>(10, "MyStringValue");

                ASSERT_EQ(NULL, oldData.getData());

                TypedData result = mixedMap->get<int>(3);
                ASSERT_EQ(-7, result.getType().typeId);
                std::auto_ptr<int> value = result.get<int>();
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                result = mixedMap->get<int>(10);
                ASSERT_EQ(-11, result.getType().typeId);
                std::auto_ptr<std::string> strValue = result.get<std::string>();
                ASSERT_NE((std::string *)NULL, strValue.get());
                ASSERT_EQ("MyStringValue", *strValue);
            }

            TEST_F(MixedMapTest, testPolymorphismWithIdentifiedDataSerializable) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                mixedMap->put<int, BaseDataSerializable>(1, base);
                mixedMap->put<int, Derived1DataSerializable>(2, derived1);
                mixedMap->put<int, Derived2DataSerializable>(3, derived2);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::auto_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *)NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
                            // serialization::pimpl::SerializationConstants::CONSTANT_TYPE_DATA, using -2 since static
                            // variable is not exported into the library
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(10, objectType.classId);
                            std::auto_ptr<BaseDataSerializable> value = valueData.get<BaseDataSerializable>();
                            ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(11, objectType.classId);
                            std::auto_ptr<BaseDataSerializable> value(valueData.get<Derived1DataSerializable>());
                            ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(12, objectType.classId);
                            std::auto_ptr<BaseDataSerializable> value(valueData.get<Derived2DataSerializable>());
                            ASSERT_NE((BaseDataSerializable *)NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }
            
            TEST_F(MixedMapTest, testPolymorphismUsingBaseClassWithIdentifiedDataSerializable) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                rawPointerMap->put(1, base);
                rawPointerMap->put(2, derived1);
                rawPointerMap->put(3, derived2);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::auto_ptr<EntryArray<int, BaseDataSerializable> > entries = rawPointerMap->getAll(keys);
                ASSERT_NE((EntryArray<int, BaseDataSerializable> *)NULL, entries.get());
                ASSERT_EQ((size_t)3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BaseDataSerializable *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *)NULL, key);
                    const BaseDataSerializable *value = entry.second;
                    ASSERT_NE((const BaseDataSerializable *)NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(base.getClassId(), value->getClassId());
                            break;
                        case 2:
                            ASSERT_EQ(derived1.getClassId(), value->getClassId());
                            break;
                        case 3:
                            ASSERT_EQ(derived2.getClassId(), value->getClassId());
                            break;
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismWithPortable) {
                BasePortable base;
                Derived1Portable derived1;
                Derived2Portable derived2;
                mixedMap->put<int, BasePortable>(1, base);
                mixedMap->put<int, Derived1Portable>(2, derived1);
                mixedMap->put<int, Derived2Portable>(3, derived2);

                TypedData secondData = mixedMap->get<int>(2);
                serialization::pimpl::ObjectType secondType = secondData.getType();
                ASSERT_EQ(-1, secondType.typeId);
                secondData.get<Derived1Portable>();
                
                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::auto_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *)NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
                            // serialization::pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE, using -1 since static
                            // variable is not exported into the library
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(3, objectType.classId);
                            std::auto_ptr<BasePortable> value = valueData.get<BasePortable>();
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(4, objectType.classId);
                            std::auto_ptr<BasePortable> value(valueData.get<Derived1Portable>());
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(5, objectType.classId);
                            std::auto_ptr<BasePortable> value(valueData.get<Derived2Portable>());
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismUsingBaseClassWithPortable) {
                BasePortable basePortable;
                Derived1Portable derived1Portable;
                Derived2Portable derived2Portable;
                rawPointerMapPortable->put(1, basePortable);
                rawPointerMapPortable->put(2, derived1Portable );
                rawPointerMapPortable->put(3, derived2Portable );

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::auto_ptr<EntryArray<int, BasePortable> > entries = rawPointerMapPortable->getAll(keys);
                ASSERT_NE((EntryArray<int, BasePortable> *)NULL, entries.get());
                ASSERT_EQ((size_t)3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BasePortable *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *)NULL, key);
                    const BasePortable *value = entry.second;
                    ASSERT_NE((const BasePortable *)NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(basePortable.getClassId(), value->getClassId());
                            break;
                        case 2:
                            ASSERT_EQ(derived1Portable.getClassId(), value->getClassId());
                            break;
                        case 3:
                            ASSERT_EQ(derived2Portable.getClassId(), value->getClassId());
                            break;
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismUsingBaseClass) {
                BaseCustom baseCustom;
                Derived1Custom derived1Custom;
                Derived2Custom derived2Custom;
                rawPointerMapCustom->put(1, baseCustom);
                rawPointerMapCustom->put(2, derived1Custom );
                rawPointerMapCustom->put(3, derived2Custom );

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::auto_ptr<EntryArray<int, BaseCustom> > entries = rawPointerMapCustom->getAll(keys);
                ASSERT_NE((EntryArray<int, BaseCustom> *)NULL, entries.get());
                ASSERT_EQ((size_t)3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BaseCustom *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *)NULL, key);
                    const BaseCustom *value = entry.second;
                    ASSERT_NE((const BaseCustom *)NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(baseCustom.getValue(), value->getValue());
                            break;
                        case 2:
                            ASSERT_EQ(derived1Custom.getValue(), value->getValue());
                            break;
                        case 3:
                            ASSERT_EQ(derived2Custom.getValue(), value->getValue());
                            break;
                        default:
                            FAIL();
                    }
                }
            }
        }
    }
}
