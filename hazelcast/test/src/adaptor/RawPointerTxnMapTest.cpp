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
// Created by ihsan demir on 24/03/16.

#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMap.h"

#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "serialization/Employee.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class RawPointerClientTxnMapTest : public ClientTestSupport {
            protected:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete clientConfig;
                    delete instance;

                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
            };

            HazelcastServer *RawPointerClientTxnMapTest::instance = NULL;
            ClientConfig *RawPointerClientTxnMapTest::clientConfig = NULL;
            HazelcastClient *RawPointerClientTxnMapTest::client = NULL;

            TEST_F(RawPointerClientTxnMapTest, testPutGet) {
                std::string name = "defMap";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> originalMap = context.getMap<std::string, std::string>(name);
                client::adaptor::RawPointerTransactionalMap<std::string, std::string> map(originalMap);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                hazelcast::util::SharedPtr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                context.commitTransaction();

                ASSERT_EQ("value1", *(client->getMap<std::string, std::string>(name).get("key1")));
            }


//            @Test MTODO
//            public void testGetForUpdate() throws TransactionException {
//            final IMap<String, Integer> map = hz.getMap("testTxnGetForUpdate");
//            final CountDownLatch latch1 = new CountDownLatch(1);
//            final CountDownLatch latch2 = new CountDownLatch(1);
//            map.put("var", 0);
//            final AtomicBoolean pass = new AtomicBoolean(true);
//
//
//            Runnable incrementor = new Runnable() {
//                public void run() {
//                    try {
//                        latch1.await(100, TimeUnit.SECONDS);
//                        pass.set(map.tryPut("var", 1, 0, TimeUnit.SECONDS) == false);
//                        latch2.countDown();
//                    } catch (Exception e) {
//                    }
//                }
//            }
//            new Thread(incrementor).start();
//            boolean b = hz.executeTransaction(new TransactionalTask<Boolean>() {
//                public Boolean execute(TransactionalTaskContext context) throws TransactionException {
//                    try {
//                        final TransactionalMap<String, Integer> txMap = context.getMap("testTxnGetForUpdate");
//                        txMap.getForUpdate("var");
//                        latch1.countDown();
//                        latch2.await(100, TimeUnit.SECONDS);
//                    } catch (Exception e) {
//                    }
//                    return true;
//                }
//            });
//            assertTrue(b);
//            assertTrue(pass.get());
//            assertTrue(map.tryPut("var", 1, 0, TimeUnit.SECONDS));
//        }

            TEST_F(RawPointerClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> originalMap = context.getMap<std::string, std::string>(name);
                client::adaptor::RawPointerTransactionalMap<std::string, std::string> txMap(originalMap);
                ASSERT_EQ(txMap.put("key3", "value3").get(), (std::string *)NULL);


                ASSERT_EQ(3, (int)txMap.size());
                ASSERT_EQ(3, (int)txMap.keySet()->size());
                ASSERT_EQ(3, (int)txMap.values()->size());
                context.commitTransaction();

                ASSERT_EQ(3, (int)map.size());
                ASSERT_EQ(3, (int)map.keySet().size());
                ASSERT_EQ(3, (int)map.values().size());

            }

            TEST_F(RawPointerClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<Employee, Employee> originalMap = context.getMap<Employee, Employee>(name);
                client::adaptor::RawPointerTransactionalMap<Employee, Employee> txMap(originalMap);

                ASSERT_EQ(txMap.put(emp2, emp2).get(), (Employee *)NULL);

                ASSERT_EQ(2, (int)txMap.size());
                ASSERT_EQ(2, (int)txMap.keySet()->size());
                query::SqlPredicate predicate("a = 10");
                ASSERT_EQ(0, (int)txMap.keySet(&predicate)->size());
                ASSERT_EQ(0, (int)txMap.values(&predicate)->size());
                query::SqlPredicate predicate2("a >= 10");
                ASSERT_EQ(2, (int)txMap.keySet(&predicate2)->size());
                ASSERT_EQ(2, (int)txMap.values(&predicate2)->size());

                context.commitTransaction();

                ASSERT_EQ(2, (int)map.size());
                ASSERT_EQ(2, (int)map.values().size());
            }
        }
    }
}

