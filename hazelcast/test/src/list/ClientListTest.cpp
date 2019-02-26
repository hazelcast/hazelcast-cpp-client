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
//
// Created by sancar koyunlu on 9/13/13.
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "hazelcast/client/HazelcastClient.h"

#include "hazelcast/client/ClientConfig.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "hazelcast/client/IList.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientListTest : public ClientTestSupport {
            protected:
                class MyListItemListener : public ItemListener<std::string> {
                public:
                    MyListItemListener(util::CountDownLatch& latch)
                            : latch(latch) {

                    }

                    void itemAdded(const ItemEvent<std::string>& itemEvent) {
                        int type = itemEvent.getEventType();
                        assertEquals((int) ItemEventType::ADDED, type);
                        assertEquals("MyList", itemEvent.getName());
                        std::string host = itemEvent.getMember().getAddress().getHost();
                        assertTrue(host == "localhost" || host == "127.0.0.1");
                        assertEquals(5701, itemEvent.getMember().getAddress().getPort());
                        assertEquals("item-1", itemEvent.getItem());
                        latch.countDown();
                    }

                    void itemRemoved(const ItemEvent<std::string>& item) {
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
                    sslFactory = new HazelcastServerFactory(getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
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
                    list = new IList<std::string>(client->getList<std::string>("MyList"));
                }

                static void TearDownTestCase() {
                    delete list;
                    delete client;
                    delete clientConfig;
                    delete instance;
                    delete sslFactory;

                    list = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static IList<std::string> *list;
                static HazelcastServerFactory *sslFactory;
            };

            HazelcastServer *ClientListTest::instance = NULL;
            ClientConfig *ClientListTest::clientConfig = NULL;
            HazelcastClient *ClientListTest::client = NULL;
            IList<std::string> *ClientListTest::list = NULL;
            HazelcastServerFactory *ClientListTest::sslFactory = NULL;

            TEST_F(ClientListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll(l));

                ASSERT_TRUE(list->addAll(1, l));
                ASSERT_EQ(4, list->size());

                ASSERT_EQ("item1", *(list->get(0)));
                ASSERT_EQ("item1", *(list->get(1)));
                ASSERT_EQ("item2", *(list->get(2)));
                ASSERT_EQ("item2", *(list->get(3)));
            }

            TEST_F(ClientListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                list->add(0, "item3");
                ASSERT_EQ(3, list->size());
                boost::shared_ptr<std::string> temp = list->set(2, "item4");
                ASSERT_EQ("item2", *temp);

                ASSERT_EQ(3, list->size());
                ASSERT_EQ("item3", *(list->get(0)));
                ASSERT_EQ("item1", *(list->get(1)));
                ASSERT_EQ("item4", *(list->get(2)));

                ASSERT_FALSE(list->remove("item2"));
                ASSERT_TRUE(list->remove("item3"));

                temp = list->remove(1);
                ASSERT_EQ("item4", *temp);

                ASSERT_EQ(1, list->size());
                ASSERT_EQ("item1", *(list->get(0)));
            }

            TEST_F(ClientListTest, testIndexOf) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                ASSERT_EQ(-1, list->indexOf("item5"));
                ASSERT_EQ(0, list->indexOf("item1"));

                ASSERT_EQ(-1, list->lastIndexOf("item6"));
                ASSERT_EQ(2, list->lastIndexOf("item1"));
            }

            TEST_F(ClientListTest, testToArray) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                std::vector<std::string> ar = list->toArray();

                ASSERT_EQ("item1", ar[0]);
                ASSERT_EQ("item2", ar[1]);
                ASSERT_EQ("item1", ar[2]);
                ASSERT_EQ("item4", ar[3]);

                std::vector<std::string> arr2 = list->subList(1, 3);

                ASSERT_EQ(2, (int)arr2.size());
                ASSERT_EQ("item2", arr2[0]);
                ASSERT_EQ("item1", arr2[1]);
            }

            TEST_F(ClientListTest, testContains) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                ASSERT_FALSE(list->contains("item3"));
                ASSERT_TRUE(list->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll(l));
                ASSERT_TRUE(list->add("item3"));
                ASSERT_TRUE(list->containsAll(l));
            }

            TEST_F(ClientListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll(l));
                ASSERT_EQ(3, (int)list->size());
                ASSERT_FALSE(list->removeAll(l));
                ASSERT_EQ(3, (int)list->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll(l));
                ASSERT_EQ(3, (int)list->size());

                l.clear();
                ASSERT_TRUE(list->retainAll(l));
                ASSERT_EQ(0, (int)list->size());

            }

            TEST_F(ClientListTest, testListener) {
                util::CountDownLatch latch(1);

                MyListItemListener listener(latch);
                std::string registrationId = list->addItemListener(listener, true);

                list->add("item-1");

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(list->removeItemListener(registrationId));
            }

            TEST_F(ClientListTest, testIsEmpty) {
                ASSERT_TRUE(list->isEmpty());
                ASSERT_TRUE(list->add("item1"));
                ASSERT_FALSE(list->isEmpty());
            }

        }
    }
}

