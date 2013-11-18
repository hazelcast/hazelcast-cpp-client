//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnTest::ClientTxnTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , second(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701)))) {
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
                instance.shutdown();
                second.shutdown();
            };

            void ClientTxnTest::beforeTest() {
            };

            void ClientTxnTest::afterTest() {
            };

            void ClientTxnTest::testTxnRollback() {
                TransactionContext context = client->newTransactionContext();
                util::CountDownLatch latch(1);
                try {
                    context.beginTransaction();
                    std::string txnId = context.getTxnId();
                    assertTrue(txnId.compare("") != 0);
                    TransactionalQueue<std::string> queue = context.getQueue<std::string>("testTxnRollback");
                    queue.offer("item");

                    instance.shutdown();

                    context.commitTransaction();
                    assertTrue(false, "commit should throw exception!!!");
                } catch (std::exception&){
                    context.rollbackTransaction();
                    latch.countDown();
                }

                assertTrue(latch.await(10 * 1000));

                IQueue<std::string> q = client->getQueue<std::string>("testTxnRollback");
                std::string actual = q.poll();
                assertEqual("", actual);
                assertEqual(0, q.size());
            }


        }
    }
}
