/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 21/03/16.

#include "hazelcast/client/adaptor/RawPointerSet.h"
#include "hazelcast/client/HazelcastClient.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerSetTest : public ClientTestSupport {
                public:
                    RawPointerSetTest()
                            : instance(*g_srvFactory)
                            , client(getNewClient())
                            , originalSet(client->getSet<std::string>("RawPointerSetTest"))
                            , set(originalSet) {
                    }

                protected:
                    class MySetItemListener : public ItemListener<std::string> {
                    public:
                        MySetItemListener(util::CountDownLatch &latch)
                                :latch(latch) {

                        }

                        void itemAdded(const ItemEvent<std::string>& itemEvent) {
                            latch.countDown();
                        }

                        void itemRemoved(const ItemEvent<std::string>& item) {
                        }

                    private:
                        util::CountDownLatch &latch;
                    };

                    HazelcastServer instance;
                    ClientConfig clientConfig;
                    std::auto_ptr<HazelcastClient> client;
                    ISet<std::string> originalSet;
                    client::adaptor::RawPointerSet<std::string> set;

                    bool itemExists(const std::vector<std::string> &items, const std::string &item) const {
                        bool found = false;
                        for (std::vector<std::string>::const_iterator it = items.begin();it != items.end();++it) {
                            if (item == *it) {
                                found = true;
                                break;
                            }
                        }
                        return found;
                    }
                };

                TEST_F(RawPointerSetTest, testAddAll) {
                    std::vector<std::string> l;
                    l.push_back("item1");
                    l.push_back("item2");

                    ASSERT_TRUE(set.addAll(l));
                    ASSERT_EQ(2, set.size());

                    ASSERT_FALSE(set.addAll(l));
                    ASSERT_EQ(2, set.size());
                }

                TEST_F(RawPointerSetTest, testAddRemove) {
                    ASSERT_TRUE(set.add("item1"));
                    ASSERT_TRUE(set.add("item2"));
                    ASSERT_TRUE(set.add("item3"));
                    ASSERT_EQ(3, set.size());

                    ASSERT_FALSE(set.add("item3"));
                    ASSERT_EQ(3, set.size());


                    ASSERT_FALSE(set.remove("item4"));
                    ASSERT_TRUE(set.remove("item3"));

                }

                TEST_F(RawPointerSetTest, testContains) {
                    ASSERT_TRUE(set.add("item1"));
                    ASSERT_TRUE(set.add("item2"));
                    ASSERT_TRUE(set.add("item3"));
                    ASSERT_TRUE(set.add("item4"));

                    ASSERT_FALSE(set.contains("item5"));
                    ASSERT_TRUE(set.contains("item2"));

                    std::vector<std::string> l;
                    l.push_back("item6");
                    l.push_back("item3");

                    ASSERT_FALSE(set.containsAll(l));
                    ASSERT_TRUE(set.add("item6"));
                    ASSERT_TRUE(set.containsAll(l));
                }

                TEST_F(RawPointerSetTest, testToArray) {
                    ASSERT_TRUE(set.add("item1"));
                    ASSERT_TRUE(set.add("item2"));
                    ASSERT_TRUE(set.add("item3"));
                    ASSERT_TRUE(set.add("item4"));
                    ASSERT_FALSE(set.add("item4"));

                    std::auto_ptr<client::adaptor::DataArray<std::string> > array = set.toArray();

                    ASSERT_EQ((size_t)4, array->size());
                    std::vector<std::string> items;

                    for (size_t i = 0; i < array->size(); ++i) {
                        std::auto_ptr<std::string> item = array->release(i);
                        ASSERT_NE((std::string *)NULL, item.get());
                        items.push_back(*item);
                    }
                    
                    ASSERT_TRUE(itemExists(items, "item1"));
                    ASSERT_TRUE(itemExists(items, "item2"));
                    ASSERT_TRUE(itemExists(items, "item3"));
                    ASSERT_TRUE(itemExists(items, "item4"));
                }

                TEST_F(RawPointerSetTest, testRemoveRetainAll) {
                    ASSERT_TRUE(set.add("item1"));
                    ASSERT_TRUE(set.add("item2"));
                    ASSERT_TRUE(set.add("item3"));
                    ASSERT_TRUE(set.add("item4"));

                    std::vector<std::string> l;
                    l.push_back("item4");
                    l.push_back("item3");

                    ASSERT_TRUE(set.removeAll(l));
                    ASSERT_EQ(2, set.size());
                    ASSERT_FALSE(set.removeAll(l));
                    ASSERT_EQ(2, set.size());

                    l.clear();
                    l.push_back("item1");
                    l.push_back("item2");
                    ASSERT_FALSE(set.retainAll(l));
                    ASSERT_EQ(2, set.size());

                    l.clear();
                    ASSERT_TRUE(set.retainAll(l));
                    ASSERT_EQ(0, set.size());
                }

                TEST_F(RawPointerSetTest, testListener) {
                    util::CountDownLatch latch(6);

                    MySetItemListener listener(latch);
                    std::string registrationId = set.addItemListener(listener, true);

                    for (int i = 0; i < 5; i++) {
                        set.add(std::string("item") + util::IOUtil::to_string(i));
                    }
                    set.add("done");

                    ASSERT_TRUE(latch.await(20 ));

                    ASSERT_TRUE(set.removeItemListener(registrationId));
                }
            }
        }
    }
}

