//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnTest::ClientTxnTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture("ClientTxnTest")
            , hazelcastInstanceFactory(hazelcastInstanceFactory) {
            };


            ClientTxnTest::~ClientTxnTest() {
            }

            void ClientTxnTest::addTests() {
                addTest(&ClientTxnTest::testTxnRollback, "testTxnRollback");
                addTest(&ClientTxnTest::testTxnWithMultipleNodes, "testTxnWithMultipleNodes");
            };

            void ClientTxnTest::beforeClass() {
            };

            void ClientTxnTest::afterClass() {
            };

            void ClientTxnTest::beforeTest() {
            };

            void ClientTxnTest::afterTest() {
                hazelcastInstanceFactory.shutdownAll();
            };


            void ClientTxnTest::testTxnWithMultipleNodes() {
                HazelcastServer server(hazelcastInstanceFactory);
                ClientConfig clientConfig;
                HazelcastClient client(clientConfig.addAddress(Address(HOST, 5701)));

                TransactionContext context = client.newTransactionContext();
                HazelcastServer secondServer(hazelcastInstanceFactory);

                bool rollbackSuccessful = false;
                try {
                    context.beginTransaction();
                    TransactionalQueue<std::string> queue = context.getQueue<std::string>("testTxnRollback");

                    server.shutdown();
                    queue.offer("item");
                    assertTrue(false, "queue offer should throw InstanceNotActiveException");

                    context.commitTransaction();
                } catch (exception::InstanceNotActiveException &) {
                    context.rollbackTransaction();
                    rollbackSuccessful = true;
                }

                assertTrue(rollbackSuccessful);

            }

            void ClientTxnTest::testTxnRollback() {
                HazelcastServer server(hazelcastInstanceFactory);
                ClientConfig clientConfig;
                HazelcastClient client(clientConfig.addAddress(Address(HOST, 5701)));

                std::auto_ptr<HazelcastServer> second;
                TransactionContext context = client.newTransactionContext();
                bool rollbackSuccessful = false;
                try {
                    context.beginTransaction();
                    second.reset(new HazelcastServer(hazelcastInstanceFactory));
                    std::string txnId = context.getTxnId();
                    assertTrue(txnId.compare("") != 0);
                    TransactionalQueue<std::string> queue = context.getQueue<std::string>("testTxnRollback");
                    queue.offer("item");
                    server.shutdown();
                    context.commitTransaction();
                    assertTrue(false, "commit should throw exception!!!");
                } catch (std::exception &) {
                    context.rollbackTransaction();
                    rollbackSuccessful = true;
                }

                assertTrue(rollbackSuccessful);

                IQueue<std::string> q = client.getQueue<std::string>("testTxnRollback");
                assertNull(q.poll().get());
                assertEqual(0, q.size());
            }


        }
    }
}
