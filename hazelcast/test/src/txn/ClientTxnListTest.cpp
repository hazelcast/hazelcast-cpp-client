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
// Created by sancar koyunlu on 9/18/13.

#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class ClientTxnListTest : public ClientTestSupport {
            public:
                ClientTxnListTest();

                ~ClientTxnListTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
            };

            ClientTxnListTest::ClientTxnListTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnListTest::~ClientTxnListTest() {
            }

            TEST_F(ClientTxnListTest, testAddRemove) {
                IList<std::string> l = client.getList<std::string>("testAddRemove");
                l.add("item1");

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalList<std::string> list = context.getList<std::string>("testAddRemove");
                ASSERT_TRUE(list.add("item2"));
                ASSERT_EQ(2, list.size());
                ASSERT_EQ(1, l.size());
                ASSERT_FALSE(list.remove("item3"));
                ASSERT_TRUE(list.remove("item1"));

                context.commitTransaction();

                ASSERT_EQ(1, l.size());
            }
        }
    }
}

