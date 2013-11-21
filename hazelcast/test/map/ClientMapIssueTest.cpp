//
// Created by sancar koyunlu on 25/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "map/ClientMapIssueTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"


namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMapIssueTest::ClientMapIssueTest(HazelcastInstanceFactory &hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory) {
            };


            void ClientMapIssueTest::addTests() {
                addTest(&ClientMapIssueTest::testClientDisconnectionWhenQuerying, "testClientDisconnectionWhenQuerying");
            }

            void ClientMapIssueTest::beforeClass() {

            }

            void ClientMapIssueTest::afterClass() {

            }

            void ClientMapIssueTest::beforeTest() {

            }

            void ClientMapIssueTest::afterTest() {

            }

            void testCDWThread(IMap<std::string, std::string> *m, util::CountDownLatch *latch) {
                try{
                    m->get("ali");
                } catch(exception::IException &){
                    latch->countDown();
                }


            }

            void ClientMapIssueTest::testClientDisconnectionWhenQuerying() {
                HazelcastInstance instance1(hazelcastInstanceFactory);
                HazelcastInstance instance2(hazelcastInstanceFactory);
                ClientConfig clientConfig;
                clientConfig.setConnectionAttemptLimit(INT_MAX);
                clientConfig.addAddress(Address("localhost", 5701));
                HazelcastClient client(clientConfig);
                IMap<std::string, std::string> m = client.getMap<std::string, std::string>("m");

                m.put("ali", "Ali");
                m.put("alev", "Alev");

                instance1.shutdown();
                instance2.shutdown();

                util::CountDownLatch latch(1);
                boost::thread t(testCDWThread, &m, &latch);
                assertTrue(latch.await(15 * 1000));
            }
        }
    }
}