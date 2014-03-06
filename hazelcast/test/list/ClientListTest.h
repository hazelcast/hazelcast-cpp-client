//
// Created by sancar koyunlu on 9/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientListTest
#define HAZELCAST_ClientListTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/IList.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientListTest : public iTest::iTestFixture<ClientListTest> {
            public:
                ClientListTest(HazelcastServerFactory &);

                ~ClientListTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAddAll();

                void testAddSetRemove();

                void testIndexOf();

                void testToArray();

                void testContains();

                void testRemoveRetainAll();

                void testListener();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IList<std::string> > list;
            };
        }
    }
}


#endif //HAZELCAST_ClientListTest
