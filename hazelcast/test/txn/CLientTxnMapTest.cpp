//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientTxnMapTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTxnMapTest::ClientTxnMapTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701)))) {
            };


            ClientTxnMapTest::~ClientTxnMapTest() {
            }

            void ClientTxnMapTest::addTests() {
                addTest(&ClientTxnMapTest::testPutGet, "testPutGet");
                addTest(&ClientTxnMapTest::testKeySetValues, "testKeySetValues");
                addTest(&ClientTxnMapTest::testKeySetAndValuesWithPredicates, "testKeysetAndValuesWithPredicates");
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
                assertEqual("", map.put("key1", "value1"));
                assertEqual("value1", map.get("key1"));
                assertEqual("", client->getMap<std::string, std::string>(name).get("key1"));
                context.commitTransaction();

                assertEqual("value1", client->getMap<std::string, std::string>(name).get("key1"));
            }


            void ClientTxnMapTest::testKeySetValues() {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> txMap = context.getMap<std::string, std::string>(name);
                assertEqual("", txMap.put("key3", "value3"));


                assertEqual(3, txMap.size());
                assertEqual(3, txMap.keySet().size());
                assertEqual(3, txMap.values().size());
                context.commitTransaction();

                assertEqual(3, map.size());
                assertEqual(3, map.keySet().size());
                assertEqual(3, map.values().size());

            }

            class Employee : public Portable {
            public:
                Employee():age(-1), name("") {

                }

                Employee(std::string name, int age)
                :age(age)
                , name(name) {

                };

                int getFactoryId() const {
                    return 666;
                };

                int getClassId() const {
                    return 2;
                };

                bool operator ==(const Employee& employee) const {
                    if (age != employee.age)
                        return false;
                    else if (name.compare(employee.name))
                        return false;
                    else
                        return true;
                }

                bool operator !=(const Employee& employee) const {
                    return !(*this == employee);
                }

                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("a", age);
                };

                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    age = reader.readInt("a");
                };

            private:
                int age;
                std::string name;
            };

            void ClientTxnMapTest::testKeySetAndValuesWithPredicates() {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                Employee empty;
                TransactionalMap<Employee, Employee> txMap = context.getMap<Employee, Employee>(name);
                assertEqual(empty, txMap.put(emp2, emp2));

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

        }
    }
}