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



#include "ClientTxnMultiMapTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnMultiMapTest::ClientTxnMultiMapTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientTxnMultiMapTest>("ClientTxnMultiMapTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient()) {
            }


            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() {
            }

            void ClientTxnMultiMapTest::addTests() {
                addTest(&ClientTxnMultiMapTest::testPutGetRemove, "testPutGetRemove");
            }

            void ClientTxnMultiMapTest::beforeClass() {
            }

            void ClientTxnMultiMapTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientTxnMultiMapTest::beforeTest() {
            }

            void ClientTxnMultiMapTest::afterTest() {
            }

            void putGetRemoveTestThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string > *mm = (MultiMap<std::string, std::string > *)args.arg0;
                HazelcastClient *client = (HazelcastClient *)args.arg1;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg2;
                util::AtomicInt *error = (util::AtomicInt *)args.arg3;
                std::string key = util::IOUtil::to_string(util::Thread::getThreadID());
                client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                TransactionContext context = client->newTransactionContext();
                try {
                    context.beginTransaction();
                    TransactionalMultiMap<std::string, std::string > multiMap = context.getMultiMap<std::string, std::string >("testPutGetRemove");
                    assertFalse(multiMap.put(key, "value"));
                    assertTrue(multiMap.put(key, "value1"));
                    assertTrue(multiMap.put(key, "value2"));
                    assertEqual(3, (int)multiMap.get(key).size());
                    context.commitTransaction();

                    assertEqual(3, (int)mm->get(key).size());

                    latch->countDown();
                } catch (std::exception &) {
                    ++(*error);
                    latch->countDown();
                } catch(iTest::iTestException&){
                    ++(*error);
                    latch->countDown();
                }
            }

            void ClientTxnMultiMapTest::testPutGetRemove() {

                MultiMap<std::string, std::string > mm = client->getMultiMap<std::string, std::string >("testPutGetRemove");
                int n = 10;
                util::CountDownLatch latch(n);
                util::AtomicInt error(0);
                std::vector<util::Thread*> threads(n);
                for (int i = 0; i < n; i++) {
                    threads[i] = new util::Thread(putGetRemoveTestThread, &mm, client.get(), &latch, &error);
                }
                assertTrue(latch.await(1));
                assertEqual(0, (int)error);
                for (int i = 0; i < n; i++) {
                    delete threads[i] ;
                }
            }


        }
    }
}

