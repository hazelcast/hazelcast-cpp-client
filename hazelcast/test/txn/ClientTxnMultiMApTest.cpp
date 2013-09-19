//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnMultiMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnMultiMapTest::ClientTxnMultiMapTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701)))) {
            };


            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() {
            }

            void ClientTxnMultiMapTest::addTests() {
                addTest(&ClientTxnMultiMapTest::testPutGetRemove, "testPutGetRemove");
            };

            void ClientTxnMultiMapTest::beforeClass() {
            };

            void ClientTxnMultiMapTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientTxnMultiMapTest::beforeTest() {
            };

            void ClientTxnMultiMapTest::afterTest() {
            };

            void putGetRemoveTestThread(MultiMap<std::string, std::string > *mm, int id, HazelcastClient *client, util::CountDownLatch *latch, boost::atomic<int> *error) {
                std::string key = util::to_string(id) + std::string("key");
                client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                TransactionContext context = client->newTransactionContext();
                try {
                    context.beginTransaction();
                    TransactionalMultiMap<std::string, std::string > multiMap = context.getMultiMap<std::string, std::string >("testPutGetRemove");
                    assertFalse(multiMap.put(key, "value"));
                    assertTrue(multiMap.put(key, "value1"));
                    assertTrue(multiMap.put(key, "value2"));
                    assertEqual(3, multiMap.get(key).size());
                    context.commitTransaction();

                    assertEqual(3, mm->get(key).size());

                    latch->countDown();
                } catch (std::exception& e) {
                    error->fetch_add(1);
                    latch->countDown();
                }
            }

            void ClientTxnMultiMapTest::testPutGetRemove() {

                MultiMap<std::string, std::string > mm = client->getMultiMap<std::string, std::string >("testPutGetRemove");
                int threads = 10;
                util::CountDownLatch latch(threads);
                boost::atomic<int> error(0);
                for (int i = 0; i < threads; i++) {
                    boost::thread t(putGetRemoveTestThread, &mm, i, client.get(), &latch, &error);
                }
                assertTrue(latch.await(1000));
                assertEqual(0, error);
            }


        }
    }
}