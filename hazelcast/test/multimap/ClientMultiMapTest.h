//
//  ClientMultiMapTest.h
//  hazelcast
//
//  Created by Sancar on 16.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientMultiMapTest_h
#define hazelcast_ClientMultiMapTest_h

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "MultiMap.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientMultiMapTest : public iTest::iTestFixture<ClientMultiMapTest> {
            public:
                ClientMultiMapTest(HazelcastInstanceFactory&);

                ~ClientMultiMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testPutGetRemove();

                void testKeySetEntrySetAndValues();

                void testContains();

                void testListener();

                void testLock();

                void testLockTtl();

                void testTryLock();

                void testForceUnlock();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<MultiMap<std::string, std::string> > mm;
            };
        }
    }
}

#endif
