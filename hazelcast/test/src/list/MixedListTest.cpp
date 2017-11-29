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

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/mixedtype/IList.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedListTest : public ClientTestSupport {
            protected:
                class MyListItemListener : public MixedItemListener {
                public:
                    MyListItemListener(util::CountDownLatch& latch)
                            : latch(latch) {

                    }

                    virtual void itemAdded(const ItemEvent<TypedData> &item) {
                        latch.countDown();
                    }

                    virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                    }
                private:
                    util::CountDownLatch& latch;
                };

                virtual void TearDown() {
                    // clear list
                    list->clear();
                }

                static void SetUpTestCase() {
                    #ifdef HZ_BUILD_WITH_SSL
                    instance = new HazelcastServer(*g_srvFactory, true);
                    #else
                    instance = new HazelcastServer(*g_srvFactory);
                    #endif

                    clientConfig = getConfig().release();

                    #ifdef HZ_BUILD_WITH_SSL
                    config::ClientNetworkConfig networkConfig;
                    config::SSLConfig sslConfig;
                    sslConfig.setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList("HIGH");
                    networkConfig.setSSLConfig(sslConfig);
                    clientConfig->setNetworkConfig(networkConfig);
                    #endif // HZ_BUILD_WITH_SSL

                    client = new HazelcastClient(*clientConfig);
                    list = new mixedtype::IList(client->getMixedList("MyMixedList"));
                }

                static void TearDownTestCase() {
                    delete list;
                    delete client;
                    delete clientConfig;
                    delete instance;

                    list = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static mixedtype::IList *list;
            };

            HazelcastServer *MixedListTest::instance = NULL;
            ClientConfig *MixedListTest::clientConfig = NULL;
            HazelcastClient *MixedListTest::client = NULL;
            mixedtype::IList *MixedListTest::list = NULL;

            TEST_F(MixedListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll<std::string>(l));

                ASSERT_TRUE(list->addAll<std::string>(1, l));
                ASSERT_EQ(4, list->size());

                ASSERT_EQ("item1", *(list->get(0).get<std::string>()));
                ASSERT_EQ("item1", *(list->get(1).get<std::string>()));
                ASSERT_EQ("item2", *(list->get(2).get<std::string>()));
                ASSERT_EQ("item2", *(list->get(3).get<std::string>()));
            }

            TEST_F(MixedListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                list->add<std::string>(0, "item3");
                ASSERT_EQ(3, list->size());
                std::auto_ptr<std::string> temp = list->set<std::string>(2, "item4").get<std::string>();
                ASSERT_EQ("item2", *temp);

                ASSERT_EQ(3, list->size());
                ASSERT_EQ("item3", *(list->get(0).get<std::string>()));
                ASSERT_EQ("item1", *(list->get(1).get<std::string>()));
                ASSERT_EQ("item4", *(list->get(2).get<std::string>()));

                ASSERT_FALSE(list->remove<std::string>("item2"));
                ASSERT_TRUE(list->remove<std::string>("item3"));

                temp = list->remove(1).get<std::string>();
                ASSERT_EQ("item4", *temp);

                ASSERT_EQ(1, list->size());
                ASSERT_EQ("item1", *(list->get(0).get<std::string>()));
            }

            TEST_F(MixedListTest, testIndexOf) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                ASSERT_EQ(-1, list->indexOf<std::string>("item5"));
                ASSERT_EQ(0, list->indexOf<std::string>("item1"));

                ASSERT_EQ(-1, list->lastIndexOf<std::string>("item6"));
                ASSERT_EQ(2, list->lastIndexOf<std::string>("item1"));
            }

            TEST_F(MixedListTest, testToArray) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                std::vector<TypedData> ar = list->toArray();

                ASSERT_EQ("item1", *ar[0].get<std::string>());
                ASSERT_EQ("item2", *ar[1].get<std::string>());
                ASSERT_EQ("item1", *ar[2].get<std::string>());
                ASSERT_EQ("item4", *ar[3].get<std::string>());

                std::vector<TypedData> arr2 = list->subList(1, 3);

                ASSERT_EQ(2, (int)arr2.size());
                ASSERT_EQ("item2", *arr2[0].get<std::string>());
                ASSERT_EQ("item1", *arr2[1].get<std::string>());
            }

            TEST_F(MixedListTest, testContains) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                ASSERT_FALSE(list->contains<std::string>("item3"));
                ASSERT_TRUE(list->contains<std::string>("item2"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll<std::string>(l));
                ASSERT_TRUE(list->add<std::string>("item3"));
                ASSERT_TRUE(list->containsAll<std::string>(l));
            }

            TEST_F(MixedListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll<std::string>(l));
                ASSERT_EQ(3, (int)list->size());
                ASSERT_FALSE(list->removeAll<std::string>(l));
                ASSERT_EQ(3, (int)list->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll<std::string>(l));
                ASSERT_EQ(3, (int)list->size());

                l.clear();
                ASSERT_TRUE(list->retainAll<std::string>(l));
                ASSERT_EQ(0, (int)list->size());

            }

            TEST_F(MixedListTest, testListener) {
                util::CountDownLatch latch(5);

                MyListItemListener listener(latch);
                std::string registrationId = list->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    list->add(std::string("item") + util::IOUtil::to_string(i));
                }

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(list->removeItemListener(registrationId));
            }
        }
    }
}

