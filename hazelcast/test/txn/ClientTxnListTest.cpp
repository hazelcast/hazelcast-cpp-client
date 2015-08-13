//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnListTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnListTest::ClientTxnListTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientTxnListTest>("ClientTxnListTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient()) {
            }


            ClientTxnListTest::~ClientTxnListTest() {
            }

            void ClientTxnListTest::addTests() {
                addTest(&ClientTxnListTest::testAddRemove, "testAddRemove");
            }

            void ClientTxnListTest::beforeClass() {
            }

            void ClientTxnListTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientTxnListTest::beforeTest() {
            }

            void ClientTxnListTest::afterTest() {
            }

            void ClientTxnListTest::testAddRemove() {
                IList<std::string> l = client->getList<std::string>("testAddRemove");
                l.add("item1");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalList<std::string> list = context.getList<std::string>("testAddRemove");
                assertTrue(list.add("item2"));
                assertEqual(2, list.size());
                assertEqual(1, l.size());
                assertFalse(list.remove("item3"));
                assertTrue(list.remove("item1"));

                context.commitTransaction();

                assertEqual(1, l.size());
            }


        }
    }
}

