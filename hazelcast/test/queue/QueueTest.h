//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_QueueTest
#define HAZELCAST_QueueTest

#include "iTest.h"
#include "ClientConfig.h"
#include "HazelcastClient.h"

using namespace iTest;
using namespace hazelcast::client;

class QueueTest : public iTestFixture<QueueTest> {
public:
    QueueTest() {

    };

    void addTests() {
        addTest(&QueueTest::testAddAll, "testAddAll");
    };

    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        q = new IQueue<std::string>(client->getQueue<std::string>("clientQueueTest"));
    };

    void afterClass() {
        delete client;
        delete q;
    };

    void beforeTest() {
        q->clear();
    };

    void afterTest() {
        q->clear();
    };

    void testAddAll() {
        std::vector<std::string > coll;
        coll.push_back("item1");
        coll.push_back("item2");
        coll.push_back("item3");
        coll.push_back("item4");

        assertTrue(q->addAll(coll));
        int size = q->size();
        assertEqual(size, coll.size());

    }

private:
    HazelcastClient *client;
    IQueue<std::string>* q;
};


#endif //HAZELCAST_QueueTest
