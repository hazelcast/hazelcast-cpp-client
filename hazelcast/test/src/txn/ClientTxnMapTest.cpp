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



#include "hazelcast/client/query/SqlPredicate.h"
#include "ClientTxnMapTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "serialization/Employee.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            ClientTxnMapTest::ClientTxnMapTest()
            : instance(*g_srvFactory)
            , client(getNewClient()) {
            }
            
            ClientTxnMapTest::~ClientTxnMapTest() {
            }

            TEST_F(ClientTxnMapTest, testPutGet) {
                std::string name = "testPutGet";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                context.commitTransaction();

                ASSERT_EQ("value1", *(client->getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testRemove) {
                std::string name = "testRemove";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_EQ((std::string *)NULL, map.remove("key2").get());
                val = map.remove("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);

                context.commitTransaction();

                IMap <std::string, std::string> regularMap = client->getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            /**
             * TODO: Enable this test when https://github.com/hazelcast/hazelcast/issues/8238 is solved
             */
            TEST_F(ClientTxnMapTest, DISABLED_testRemoveIfSame) {
                std::string name = "testRemoveIfSame";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_EQ((std::string *)NULL, map.remove("key2").get());
                ASSERT_TRUE(map.remove("key1", "value1"));

                context.commitTransaction();

                IMap <std::string, std::string> regularMap = client->getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            TEST_F(ClientTxnMapTest, testDeleteEntry) {
                std::string name = "testDeleteEntry";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_NO_THROW(map.deleteEntry("key1"));

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_NO_THROW(map.deleteEntry("key1"));
                val = map.get("key1");
                ASSERT_EQ((std::string *)NULL, val.get());

                context.commitTransaction();

                IMap <std::string, std::string> regularMap = client->getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            TEST_F(ClientTxnMapTest, testReplace) {
                std::string name = "testReplace";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_EQ("value1", *map.replace("key1", "myNewValue"));

                context.commitTransaction();

                ASSERT_EQ("myNewValue", *(client->getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testSet) {
                std::string name = "testSet";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_NO_THROW(map.set("key1", "value1"));

                boost::shared_ptr<std::string> val = map.get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);

                val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_NO_THROW(map.set("key1", "myNewValue"));

                val = map.get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("myNewValue", *val);

                context.commitTransaction();

                val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("myNewValue", *val);
            }

            TEST_F(ClientTxnMapTest, testContains) {
                std::string name = "testContains";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_FALSE(map.containsKey("key1"));

                ASSERT_NO_THROW(map.set("key1", "value1"));

                boost::shared_ptr<std::string> val = map.get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);

                ASSERT_TRUE(map.containsKey("key1"));

                context.commitTransaction();

                IMap <std::string, std::string> regularMap = client->getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.containsKey("key1"));
            }

            TEST_F(ClientTxnMapTest, testReplaceIfSame) {
                std::string name = "testReplaceIfSame";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *)NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                boost::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                ASSERT_FALSE(map.replace("key1", "valueNonExistent", "myNewValue"));
                ASSERT_FALSE(map.replace("key1", "value1", "myNewValue"));

                context.commitTransaction();

                ASSERT_EQ("myNewValue", *(client->getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testPutIfSame) {
                std::string name = "testPutIfSame";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                boost::shared_ptr<std::string> val = map.putIfAbsent("key1", "value1");
                ASSERT_EQ((std::string *)NULL, val.get());
                val = map.get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);
                val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *)NULL);

                val = map.putIfAbsent("key1", "value1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);

                context.commitTransaction();

                val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value1", *val);
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

            TEST_F(ClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> txMap = context.getMap<std::string, std::string>(name);
                ASSERT_EQ(txMap.put("key3", "value3").get(), (std::string *)NULL);


                ASSERT_EQ(3, (int)txMap.size());
                ASSERT_EQ(3, (int)txMap.keySet().size());
                ASSERT_EQ(3, (int)txMap.values().size());
                context.commitTransaction();

                ASSERT_EQ(3, (int)map.size());
                ASSERT_EQ(3, (int)map.keySet().size());
                ASSERT_EQ(3, (int)map.values().size());

            }

            TEST_F(ClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<Employee, Employee> txMap = context.getMap<Employee, Employee>(name);
                ASSERT_EQ(txMap.put(emp2, emp2).get(), (Employee *)NULL);

                ASSERT_EQ(2, (int)txMap.size());
                ASSERT_EQ(2, (int)txMap.keySet().size());
                query::SqlPredicate predicate("a = 10");
                ASSERT_EQ(0, (int)txMap.keySet(&predicate).size());
                ASSERT_EQ(0, (int)txMap.values(&predicate).size());
                query::SqlPredicate predicate2("a >= 10");
                ASSERT_EQ(2, (int)txMap.keySet(&predicate2).size());
                ASSERT_EQ(2, (int)txMap.values(&predicate2).size());

                context.commitTransaction();

                ASSERT_EQ(2, (int)map.size());
                ASSERT_EQ(2, (int)map.values().size());
            }
        }
    }
}

