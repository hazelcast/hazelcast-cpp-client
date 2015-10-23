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
// Created by sancar koyunlu on 9/18/13.



#include "ClientTxnSetTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnSetTest::ClientTxnSetTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientTxnSetTest>("ClientTxnSetTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient()) {
            }


            ClientTxnSetTest::~ClientTxnSetTest() {
            }

            void ClientTxnSetTest::addTests() {
                addTest(&ClientTxnSetTest::testAddRemove, "testAddRemove");
            }

            void ClientTxnSetTest::beforeClass() {
            }

            void ClientTxnSetTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientTxnSetTest::beforeTest() {
            }

            void ClientTxnSetTest::afterTest() {
            }

            void ClientTxnSetTest::testAddRemove() {
                ISet<std::string> s = client->getSet<std::string>("testAddRemove");
                s.add("item1");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalSet<std::string> set = context.getSet<std::string>("testAddRemove");
                assertTrue(set.add("item2"));
                assertEqual(2, set.size());
                assertEqual(1, s.size());
                assertFalse(set.remove("item3"));
                assertTrue(set.remove("item1"));

                context.commitTransaction();

                assertEqual(1, s.size());
            }


        }
    }
}

