//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnTest::ClientTxnTest(HazelcastServerFactory &hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , iTestFixture("ClientTxnTest")
            , server(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701)))) {
            };


            ClientTxnTest::~ClientTxnTest() {
            }

            void ClientTxnTest::addTests() {
                addTest(&ClientTxnTest::testTxnRollback, "testTxnRollback");
            };

            void ClientTxnTest::beforeClass() {
            };

            void ClientTxnTest::afterClass() {
                client.reset();
                server.shutdown();
            };

            void ClientTxnTest::beforeTest() {
            };

            void ClientTxnTest::afterTest() {
            };

            void ClientTxnTest::testTxnRollback() {
                std::auto_ptr<HazelcastServer> second;
                TransactionContext context = client->newTransactionContext();
                util::CountDownLatch latch(1);
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
                    latch.countDown();
                }

                assertTrue(latch.await(10 * 1000));

                IQueue<std::string> q = client->getQueue<std::string>("testTxnRollback");
                assertNull(q.poll().get());
                assertEqual(0, q.size());
            }


        }
    }
}
