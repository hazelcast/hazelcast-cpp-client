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
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MySetItemListener : public MixedItemListener {
            public:
                MySetItemListener(util::CountDownLatch &latch)
                        :latch(latch) {
                }

                virtual void itemAdded(const ItemEvent<TypedData> &item) {
                    latch.countDown();
                }

                virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                }

            private:
                util::CountDownLatch &latch;
            };

            class MixedSetTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    set->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient;
                    set = new mixedtype::ISet(client->toMixedType().getSet("MySet"));
                }

                static void TearDownTestCase() {
                    delete set;
                    delete client;
                    delete instance;

                    set = NULL;
                    client = NULL;
                    instance = NULL;
                }

                bool itemExists(const std::vector<TypedData> &items, const std::string &item) const {
                    bool found = false;
                    for (std::vector<TypedData>::const_iterator it = items.begin();it != items.end();++it) {
                        if (item == *((*it).get<std::string>())) {
                            found = true;
                            break;
                        }
                    }
                    return found;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static  mixedtype::ISet *set;
            };

            HazelcastServer *MixedSetTest::instance = NULL;
            HazelcastClient *MixedSetTest::client = NULL;
             mixedtype::ISet *MixedSetTest::set = NULL;

            TEST_F(MixedSetTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");

                ASSERT_TRUE(set->addAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                ASSERT_FALSE(set->addAll<std::string>(l));
                ASSERT_EQ(2, set->size());
            }

            TEST_F(MixedSetTest, testAddRemove) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_EQ(3, set->size());

                ASSERT_FALSE(set->add<std::string>("item3"));
                ASSERT_EQ(3, set->size());


                ASSERT_FALSE(set->remove<std::string>("item4"));
                ASSERT_TRUE(set->remove<std::string>("item3"));
            }

            TEST_F(MixedSetTest, testContains) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));

                ASSERT_FALSE(set->contains<std::string>("item5"));
                ASSERT_TRUE(set->contains<std::string>("item2"));

                std::vector<std::string> l;
                l.push_back("item6");
                l.push_back("item3");

                ASSERT_FALSE(set->containsAll<std::string>(l));
                ASSERT_TRUE(set->add<std::string>("item6"));
                ASSERT_TRUE(set->containsAll<std::string>(l));
            }

            TEST_F(MixedSetTest, testToArray) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));
                ASSERT_FALSE(set->add<std::string>("item4"));

                std::vector<TypedData> items = set->toArray();

                ASSERT_EQ((size_t)4, items.size());
                ASSERT_TRUE(itemExists(items, "item1"));
                ASSERT_TRUE(itemExists(items, "item2"));
                ASSERT_TRUE(itemExists(items, "item3"));
                ASSERT_TRUE(itemExists(items, "item4"));
            }
            
            TEST_F(MixedSetTest, testRemoveRetainAll) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(set->removeAll<std::string>(l));
                ASSERT_EQ(2, set->size());
                ASSERT_FALSE(set->removeAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(set->retainAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                ASSERT_TRUE(set->retainAll<std::string>(l));
                ASSERT_EQ(0, set->size());

            }
            
            TEST_F(MixedSetTest, testListener) {
                util::CountDownLatch latch(6);

                MySetItemListener listener(latch);
                std::string registrationId = set->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    set->add<std::string>(std::string("item") + util::IOUtil::to_string(i));
                }
                set->add<std::string>("done");

                ASSERT_TRUE(latch.await(20 ));

                ASSERT_TRUE(set->removeItemListener(registrationId));
            }
        }
    }
}

