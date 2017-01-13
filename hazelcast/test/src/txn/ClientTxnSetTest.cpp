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
// Created by sancar koyunlu on 9/18/13.

#include "ClientTxnSetTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            ClientTxnSetTest::ClientTxnSetTest()
            : instance(*g_srvFactory)
            , client(getNewClient()) {
            }
            
            ClientTxnSetTest::~ClientTxnSetTest() {
            }

            TEST_F(ClientTxnSetTest, testAddRemove) {
                ISet<std::string> s = client->getSet<std::string>("testAddRemove");
                s.add("item1");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalSet<std::string> set = context.getSet<std::string>("testAddRemove");
                ASSERT_TRUE(set.add("item2"));
                ASSERT_EQ(2, set.size());
                ASSERT_EQ(1, s.size());
                ASSERT_FALSE(set.remove("item3"));
                ASSERT_TRUE(set.remove("item1"));

                context.commitTransaction();

                ASSERT_EQ(1, s.size());
            }
        }
    }
}

