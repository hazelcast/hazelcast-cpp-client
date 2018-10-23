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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include <vector>

#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapTest : public ClientTestSupport {
            public:
            protected:
                static const int OPERATION_COUNT;

                std::vector<std::pair<int, int> > buildTestValues() {
                    std::vector<std::pair<int, int> > testValues;
                    for (int i = 0; i < 100; ++i) {
                        testValues.push_back(std::make_pair(rand(), rand()));
                    }
                    return testValues;
                }

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance1 = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    client2 = new HazelcastClient(*clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete clientConfig;
                    delete instance1;
                    delete instance2;

                    client = NULL;
                    client2 = NULL;
                    clientConfig = NULL;
                    instance1 = NULL;
                    instance2 = NULL;
                }

                static HazelcastServer *instance1;
                static HazelcastServer *instance2;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static HazelcastClient *client2;
            };

            HazelcastServer *ClientReplicatedMapTest::instance1 = NULL;
            HazelcastServer *ClientReplicatedMapTest::instance2 = NULL;
            ClientConfig *ClientReplicatedMapTest::clientConfig = NULL;
            HazelcastClient *ClientReplicatedMapTest::client = NULL;
            HazelcastClient *ClientReplicatedMapTest::client2 = NULL;
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
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::map<std::string, std::string> mapTest;
                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    mapTest[out.str()] = "bar";
                }
                map1->putAll(mapTest);
                ASSERT_EQ((int32_t) mapTest.size(), map1->size());
                boost::shared_ptr<EntryArray<std::string, std::string> > entries = map1->entrySet();
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
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map1 = client->getReplicatedMap<std::string, std::string>(getTestName());
                boost::shared_ptr<ReplicatedMap<std::string, std::string> > map2 = client->getReplicatedMap<std::string, std::string>(getTestName());

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
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<EntryArray<std::string, std::string> > entries = map2->entrySet();
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
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<EntryArray<std::string, std::string> > entries = map2->entrySet();
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
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<EntryArray<std::string, std::string> > entries = map2->entrySet();
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
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                boost::shared_ptr<EntryArray<std::string, std::string> > entries = map2->entrySet();
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
                boost::shared_ptr<ReplicatedMap<int, int> > map2 = client->getReplicatedMap<int, int>(getTestName());


                std::vector<std::pair<int, int> > testValues = buildTestValues();
                size_t half = testValues.size() / 2;

                for (size_t i = 0; i < testValues.size(); i++) {
                    boost::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                ASSERT_EQ((int32_t) testValues.size(), map1->size());
                ASSERT_EQ((int32_t) testValues.size(), map2->size());
            }

        }
    }
}
