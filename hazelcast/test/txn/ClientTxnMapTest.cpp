//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "serialization/Employee.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnMapTest::ClientTxnMapTest(HazelcastInstanceFactory &hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701)))) {
            };


            ClientTxnMapTest::~ClientTxnMapTest() {
            }

            void ClientTxnMapTest::addTests() {
                addTest(&ClientTxnMapTest::testPutGet, "testPutGet");
                addTest(&ClientTxnMapTest::testKeySetValues, "testKeySetValues");
                addTest(&ClientTxnMapTest::testKeySetAndValuesWithPredicates, "testKeysetAndValuesWithPredicates");
                addTest(&ClientTxnMapTest::testExecuteTxn, "testExecuteTxn");
                addTest(&ClientTxnMapTest::testExecuteTxnWithException, "testExecuteTxnWithException");

            };

            void ClientTxnMapTest::beforeClass() {
            };

            void ClientTxnMapTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientTxnMapTest::beforeTest() {
            };

            void ClientTxnMapTest::afterTest() {
            };

            void ClientTxnMapTest::testPutGet() {
                std::string name = "defMap";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                assertNull(map.put("key1", "value1").get());
                assertEqual("value1", *(map.get("key1")));
                assertNull(client->getMap<std::string, std::string>(name).get("key1").get());

                context.commitTransaction();

                assertEqual("value1", *(client->getMap<std::string, std::string>(name).get("key1")));
            }


//            @Test TODO
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
//            };
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

            void ClientTxnMapTest::testKeySetValues() {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> txMap = context.getMap<std::string, std::string>(name);
                assertNull(txMap.put("key3", "value3").get());


                assertEqual(3, txMap.size());
                assertEqual(3, txMap.keySet().size());
                assertEqual(3, txMap.values().size());
                context.commitTransaction();

                assertEqual(3, map.size());
                assertEqual(3, map.keySet().size());
                assertEqual(3, map.values().size());

            }

            void ClientTxnMapTest::testKeySetAndValuesWithPredicates() {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<Employee, Employee> txMap = context.getMap<Employee, Employee>(name);
                assertNull(txMap.put(emp2, emp2).get());

                assertEqual(2, txMap.size());
                assertEqual(2, txMap.keySet().size());
                assertEqual(0, txMap.keySet("age = 10").size());
                assertEqual(0, txMap.values("age = 10").size());
                assertEqual(2, txMap.keySet("age >= 10").size());
                assertEqual(2, txMap.values("age >= 10").size());

                context.commitTransaction();

                assertEqual(2, map.size());
                assertEqual(2, map.values().size());


            }

            class SimpleTxnTask {
            public:
                bool execute(TransactionalTaskContext &context) const {
                    TransactionalMap<string, string> map = context.getMap<string, string>("testExecuteTxn");
                    map.put("key1", "val1");
                    return true;
                }
            };

            void ClientTxnMapTest::testExecuteTxn() {

                SimpleTxnTask task;

                bool res = client->executeTransaction<bool>(task);

                IMap<string, string> map = client->getMap<string, string>("testExecuteTxn");

                assertTrue(res);

                assertEqual("val1", *(map.get("key1")));
                assertEqual(1, map.size());
            }


            class SimpleTnxTaskFail {
            public:
                bool execute(TransactionalTaskContext &context) const {
                    TransactionalMap<string, string> map = context.getMap<string, string>("testExecuteTxnWithException");
                    map.put("key1", "Val1");
                    map.put("key2", "Val2");
                    map.put("key3", "Val3");
                    throw  std::exception();
                }
            };

            void ClientTxnMapTest::testExecuteTxnWithException() {
                SimpleTnxTaskFail task;
                try {
                    client->executeTransaction<bool>(task);
                } catch(std::exception &e) {}

                IMap<string, string> map = client->getMap<string, string>("testExecuteTxnWithException");

                assertNull(map.get("key1").get());
                assertEqual(0, map.size());
            }
        }
    }
}