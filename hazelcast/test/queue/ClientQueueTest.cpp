//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientQueueTest.h"
#include "HazelcastClient.h"
#include "HazelcastInstanceFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientQueueTest::ClientQueueTest(HazelcastInstanceFactory & hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory.newHazelcastInstance())
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , q(new IQueue< std::string>(client->getQueue< std::string >("clientQueueTest"))) {

            }

            ClientQueueTest::~ClientQueueTest() {

            }

            void ClientQueueTest::addTests() {
                addTest(&ClientQueueTest::testAddAll, "testAddAll");
            }

            void ClientQueueTest::beforeClass() {

            }

            void ClientQueueTest::afterClass() {

            }

            void ClientQueueTest::beforeTest() {
                q->clear();
            }

            void ClientQueueTest::afterTest() {
                q->clear();
            }

            void ClientQueueTest::testAddAll() {
                std::vector<std::string > coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                assertTrue(q->addAll(coll));
                int size = q->size();
                assertEqual(size, coll.size());

            }
        }
    }
}