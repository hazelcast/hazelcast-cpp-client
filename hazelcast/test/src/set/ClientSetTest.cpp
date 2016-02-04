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
// Created by sancar koyunlu on 9/13/13.

#include "ClientSetTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ClientSetTest::ClientSetTest()
            : instance(*g_srvFactory)
            , client(getNewClient())
            , set(new ISet<std::string >(client->getSet< std::string >("ClientSetTest"))) {
            }


            ClientSetTest::~ClientSetTest() {
            }

            TEST_F(ClientSetTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");

                ASSERT_TRUE(set->addAll(l));
                ASSERT_EQ(2, set->size());

                ASSERT_FALSE(set->addAll(l));
                ASSERT_EQ(2, set->size());

            }

            TEST_F(ClientSetTest, testAddRemove) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_EQ(3, set->size());

                ASSERT_FALSE(set->add("item3"));
                ASSERT_EQ(3, set->size());


                ASSERT_FALSE(set->remove("item4"));
                ASSERT_TRUE(set->remove("item3"));

            }

            TEST_F(ClientSetTest, testContains) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_TRUE(set->add("item4"));

                ASSERT_FALSE(set->contains("item5"));
                ASSERT_TRUE(set->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item6");
                l.push_back("item3");

                ASSERT_FALSE(set->containsAll(l));
                ASSERT_TRUE(set->add("item6"));
                ASSERT_TRUE(set->containsAll(l));
            }


            TEST_F(ClientSetTest, testRemoveRetainAll) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_TRUE(set->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(set->removeAll(l));
                ASSERT_EQ(2, set->size());
                ASSERT_FALSE(set->removeAll(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(set->retainAll(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                ASSERT_TRUE(set->retainAll(l));
                ASSERT_EQ(0, set->size());

            }

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


            TEST_F(ClientSetTest, testListener) {
                util::CountDownLatch latch(6);

                MySetItemListener listener(latch);
                std::string registrationId = set->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    set->add(std::string("item") + util::IOUtil::to_string(i));
                }
                set->add("done");

                ASSERT_TRUE(latch.await(20 ));

                ASSERT_TRUE(set->removeItemListener(registrationId));
            }

        }
    }
}

