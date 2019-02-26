/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/foreach.hpp>

#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapTest : public ClientTestSupport {
            public:
            protected:
                static const int OPERATION_COUNT;

                typedef std::vector<std::pair<int, int> > TEST_VALUES_TYPE;

                class SamplePortable : public serialization::Portable {
                public:
                    SamplePortable(int a) : a(a) {}

                    SamplePortable() {}

                    virtual int getFactoryId() const {
                        return 5;
                    }

                    virtual int getClassId() const {
                        return 6;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                        writer.writeInt("a", a);
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                        a = reader.readInt("a");
                    }

                    int32_t a;
                };

                class SamplePortableFactory : public serialization::PortableFactory {
                public:
                    virtual auto_ptr<serialization::Portable> create(int32_t classId) const {
                        return auto_ptr<serialization::Portable>(new SamplePortable());
                    }
                };

                bool findValueForKey(int key, TEST_VALUES_TYPE &testValues, int &value) {
                    BOOST_FOREACH(const TEST_VALUES_TYPE::value_type &entry, testValues) {
                                    if (key == entry.first) {
                                        value = entry.second;
                                        return true;
                                    }
                                }
                    return false;
                }

                template<typename T>
                bool contains(boost::shared_ptr<DataArray<T> > &values, const T &value) {
                    for (size_t i = 0; i < values->size(); ++i) {
                        if (*values->get(i) == value) {
                            return true;
                        }
                    }
                    return false;
                }

                TEST_VALUES_TYPE buildTestValues() {
                    TEST_VALUES_TYPE testValues;
                    for (int i = 0; i < 100; ++i) {
                        testValues.push_back(std::make_pair(i, i * i));
                    }
                    return testValues;
                }

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory("hazelcast/test/resources/replicated-map-binary-in-memory-config-hazelcast.xml");
                    instance1 = new HazelcastServer(*factory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(factory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    client2 = new HazelcastClient(*clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete clientConfig;
                    delete instance1;
                    delete factory;

                    client = NULL;
                    client2 = NULL;
                    clientConfig = NULL;
                    instance1 = NULL;
                    factory = NULL;
                }

                static ClientConfig getClientConfigWithNearCacheInvalidationEnabled() {
                    boost::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(
                            new config::NearCacheConfig<int, int>());
                    nearCacheConfig->setInvalidateOnChange(true).setInMemoryFormat(config::BINARY);
                    return ClientConfig().addNearCacheConfig(nearCacheConfig);
                }

                static HazelcastServer *instance1;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static HazelcastClient *client2;
                static HazelcastServerFactory *factory;
            };

            HazelcastServer *ClientReplicatedMapTest::instance1 = NULL;
            ClientConfig *ClientReplicatedMapTest::clientConfig = NULL;
            HazelcastClient *ClientReplicatedMapTest::client = NULL;
            HazelcastClient *ClientReplicatedMapTest::client2 = NULL;
            HazelcastServerFactory *ClientReplicatedMapTest::factory = NULL;
            const int ClientReplicatedMapTest::OPERATION_COUNT = 100;

            TEST_F(ClientReplicatedMapTest, testEmptyMapIsEmpty) {
                boost::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                ASSERT_TRUE(map->isEmpty()) << "map should be empty";
            }

            TEST_F(ClientReplicatedMapTest, testNonEmptyMapIsNotEmpty) {
                boost::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                map->put(1, 1);
                ASSERT_FALSE(map->isEmpty()) << "map should not be empty";
            }

            TEST_F(ClientReplicatedMapTest, testPutAll) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                std::map<std::string, std::string> mapTest;
                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    mapTest[out.str()] = "bar";
                }
                map1->putAll(mapTest);
                ASSERT_EQ((int32_t) mapTest.size(), map1->size());
                boost::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                // TODO add server side data check using remote controller scripting
            }

            TEST_F(ClientReplicatedMapTest, testGet) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 = client->getReplicatedMap<std::string, std::string>(
                        getTestName());
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 = client2->getReplicatedMap<std::string, std::string>(
                        getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_NOTNULL(map1->get(out.str()).get(), std::string);
                    ASSERT_NOTNULL(map2->get(out.str()).get(), std::string);
                    ASSERT_EQ("bar", *map1->get(out.str()));
                    ASSERT_EQ("bar", *map2->get(out.str()));
                }
            }

            TEST_F(ClientReplicatedMapTest, testPutNullReturnValueDeserialization) {
                boost::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                ASSERT_NULL("Put should return null", map->put(1, 2).get(), int);
            }

            TEST_F(ClientReplicatedMapTest, testPutReturnValueDeserialization) {
                boost::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                map->put(1, 2);

                boost::shared_ptr<int> value = map->put(1, 3);
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientReplicatedMapTest, testAdd) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }
            }

            TEST_F(ClientReplicatedMapTest, testClear) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                map1->clear();
                ASSERT_EQ(0, map1->size());
                ASSERT_EQ(0, map2->size());
            }

            TEST_F(ClientReplicatedMapTest, testUpdate) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map2->put(out.str(), "bar2");
                }

                entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar2", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar2", *value);
                }

            }

            TEST_F(ClientReplicatedMapTest, testRemove) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    boost::shared_ptr<std::string> value = map2->remove(out.str());
                    ASSERT_NOTNULL(value.get(), std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_NULL("Removed value should not exist for map1", map1->get(out.str()).get(), std::string);
                    ASSERT_NULL("Removed value should not exist for map2", map2->get(out.str()).get(), std::string);
                }
            }

            TEST_F(ClientReplicatedMapTest, testSize) {
                boost::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                ASSERT_EQ((int32_t) testValues.size(), map1->size());
                ASSERT_EQ((int32_t) testValues.size(), map2->size());
            }

            TEST_F(ClientReplicatedMapTest, testContainsKey) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_TRUE(map2->containsKey(out.str()));
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_TRUE(map1->containsKey(out.str()));
                }
            }

            TEST_F(ClientReplicatedMapTest, testContainsValue) {
                boost::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                BOOST_FOREACH(TEST_VALUES_TYPE::value_type & entry, testValues) {
                                ASSERT_TRUE(map2->containsValue(entry.second));
                            }

                BOOST_FOREACH(TEST_VALUES_TYPE::value_type & entry, testValues) {
                                ASSERT_TRUE(map1->containsValue(entry.second));
                            }
            }

            TEST_F(ClientReplicatedMapTest, testValues) {
                boost::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                boost::shared_ptr<DataArray<int> > values1 = map1->values();
                boost::shared_ptr<DataArray<int> > values2 = map2->values();

                BOOST_FOREACH(TEST_VALUES_TYPE::value_type & entry, testValues) {
                                ASSERT_TRUE(contains(values1, entry.second));
                                ASSERT_TRUE(contains(values2, entry.second));
                            }
            }

            TEST_F(ClientReplicatedMapTest, testKeySet) {
                boost::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                boost::shared_ptr<DataArray<int> > keys1 = map1->keySet();
                boost::shared_ptr<DataArray<int> > keys2 = map2->keySet();

                BOOST_FOREACH(TEST_VALUES_TYPE::value_type & entry, testValues) {
                                ASSERT_TRUE(contains(keys1, entry.first));
                                ASSERT_TRUE(contains(keys2, entry.first));
                            }
            }

            TEST_F(ClientReplicatedMapTest, testEntrySet) {
                boost::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                boost::shared_ptr<LazyEntryArray<int, int> > entrySet1 = map1->entrySet();
                boost::shared_ptr<LazyEntryArray<int, int> > entrySet2 = map2->entrySet();

                for (size_t j = 0; j < entrySet2->size(); ++j) {
                    int value;
                    ASSERT_TRUE(findValueForKey(*entrySet2->getKey(j), testValues, value));
                    ASSERT_EQ(value, *entrySet2->getValue(j));
                }

                for (size_t j = 0; j < entrySet1->size(); ++j) {
                    int value;
                    ASSERT_TRUE(findValueForKey(*entrySet1->getKey(j), testValues, value));
                    ASSERT_EQ(value, *entrySet1->getValue(j));
                }
            }

            TEST_F(ClientReplicatedMapTest, testRetrieveUnknownValue) {
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                boost::shared_ptr<std::string> value = map->get("foo");
                ASSERT_NULL("No entry with key foo should exist", value.get(), std::string);
            }

            TEST_F(ClientReplicatedMapTest, testNearCacheInvalidation) {
                std::string mapName = randomString();

                ClientConfig clientConfig = getClientConfigWithNearCacheInvalidationEnabled();
                HazelcastClient client1(clientConfig);
                HazelcastClient client2(clientConfig);

                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap1 = client1.getReplicatedMap<int, int>(
                        mapName);

                replicatedMap1->put(1, 1);
                // puts key 1 to Near Cache
                replicatedMap1->get(1);

                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap2 = client2.getReplicatedMap<int, int>(
                        mapName);
                // this should invalidate Near Cache of replicatedMap1
                replicatedMap2->clear();

                ASSERT_NULL_EVENTUALLY(replicatedMap1->get(1).get(), int);
            }

            TEST_F(ClientReplicatedMapTest, testClientPortableWithoutRegisteringToNode) {
                ClientConfig clientConfig;
                SerializationConfig serializationConfig;
                serializationConfig.addPortableFactory(5, boost::shared_ptr<serialization::PortableFactory>(
                        new SamplePortableFactory()));
                clientConfig.setSerializationConfig(serializationConfig);

                HazelcastClient client(clientConfig);
                boost::shared_ptr<ReplicatedMap<int, SamplePortable> > sampleMap = client.getReplicatedMap<int, SamplePortable>(
                        getTestName());
                sampleMap->put(1, SamplePortable(666));
                boost::shared_ptr<SamplePortable> samplePortable = sampleMap->get(1);
                ASSERT_NOTNULL(samplePortable.get(), SamplePortable);
                ASSERT_EQ(666, samplePortable->a);
            }
        }
    }
}
