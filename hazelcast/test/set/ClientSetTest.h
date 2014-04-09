//
// Created by sancar koyunlu on 9/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientSetTest
#define HAZELCAST_ClientSetTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/ISet.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientSetTest : public iTest::iTestFixture<ClientSetTest> {
            public:
                ClientSetTest(HazelcastServerFactory &);

                ~ClientSetTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAddAll();

                void testAddRemove();

                void testContains();

                void testRemoveRetainAll();

                void testListener();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ISet<std::string> > set;
            };
        }
    }
}

#endif //HAZELCAST_ClientSetTest

