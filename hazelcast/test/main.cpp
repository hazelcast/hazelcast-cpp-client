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
#include "SimpleMapTest.h"
#include "HazelcastServerFactory.h"
#include "queue/ClientQueueTest.h"
#include "multimap/ClientMultiMapTest.h"
#include "map/ClientMapTest.h"
#include "map/ClientExpirationListenerTest.h"
#include "serialization/ClientSerializationTest.h"
#include "protocol/ClientMessageTest.h"
#include "list/ClientListTest.h"
#include "set/ClientSetTest.h"
#include "atomiclong/IAtomicLongTest.h"
#include "topic/ClientTopicTest.h"
#include "idgenerator/IdGeneratorTest.h"
#include "countdownlatch/ICountDownLatchTest.h"
#include "lock/ClientLockTest.h"
#include "semaphore/ClientSemaphoreTest.h"
#include "txn/ClientTxnTest.h"
#include "txn/ClientTxnSetTest.h"
#include "txn/ClientTxnQueueTest.h"
#include "txn/ClientTxnMapTest.h"
#include "txn/ClientTxnListTest.h"
#include "txn/ClientTxnMultiMapTest.h"
#include "cluster/ClusterTest.h"
#include "cluster/MemberAttributeTest.h"
#include "issues/IssueTest.h"
#include "util/ClientUtilTest.h"
#include "util/BitsTest.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

using namespace hazelcast::client::test;

void testSpeed(const char* address) {
    SimpleMapTest s(address, 5701);
    s.run();
}

int unitTests(const char* address) {
    try {
        RUN_TEST_NO_ARGS(ClientUtilTest);
        RUN_TEST_NO_ARGS(ClientSerializationTest);
        RUN_TEST_NO_ARGS(util::BitsTest);
        RUN_TEST_NO_ARGS(test::protocol::ClientMessageTest);
        HazelcastServerFactory factory(address);
        RUN_TEST(ClientMapTest, factory);
        RUN_TEST(ClientExpirationListenerTest, factory);
        RUN_TEST(IssueTest, factory);
        RUN_TEST(MemberAttributeTest, factory);
        RUN_TEST(ClusterTest, factory);
        RUN_TEST(ClientMultiMapTest, factory);
        RUN_TEST(ClientQueueTest, factory);
        RUN_TEST(ClientListTest, factory);
        RUN_TEST(ClientSetTest, factory);
        RUN_TEST(IAtomicLongTest, factory);
        RUN_TEST(IdGeneratorTest, factory);
        RUN_TEST(ICountDownLatchTest, factory);
        RUN_TEST(ClientLockTest, factory);
        RUN_TEST(ClientSemaphoreTest, factory);
        RUN_TEST(ClientTopicTest, factory);
        RUN_TEST(ClientTxnListTest, factory);
        RUN_TEST(ClientTxnMapTest, factory);
        RUN_TEST(ClientTxnMultiMapTest, factory);
        RUN_TEST(ClientTxnQueueTest, factory);
        RUN_TEST(ClientTxnSetTest, factory);
        RUN_TEST(ClientTxnTest, factory);
        return 0;
    } catch (std::exception& e) {
        std::cout << "unitTests " << e.what() << std::endl;
        return 1;
    }
}

int main(int argc, char** argv) {
    const char* address;
    if(argc == 2){
        address = argv[1];
    } else {
        address = "127.0.0.1";
    }
    std::cout << "Server address : "  << address << std::endl;

//    testSpeed(address);
    return unitTests(address);
}


