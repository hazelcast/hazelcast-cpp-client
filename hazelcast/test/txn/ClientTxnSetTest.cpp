//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnSetTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnSetTest::ClientTxnSetTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture<ClientTxnSetTest>("ClientTxnSetTest")
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701)))) {
            };


            ClientTxnSetTest::~ClientTxnSetTest() {
            }

            void ClientTxnSetTest::addTests() {
                addTest(&ClientTxnSetTest::testAddRemove, "testAddRemove");
            };

            void ClientTxnSetTest::beforeClass() {
            };

            void ClientTxnSetTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientTxnSetTest::beforeTest() {
            };

            void ClientTxnSetTest::afterTest() {
            };

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

