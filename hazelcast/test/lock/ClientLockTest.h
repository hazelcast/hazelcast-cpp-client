//
//  ClientLockTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 15.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientLockTest_h
#define hazelcast_ClientLockTest_h

#include "iTest.h"
#include "ClientConfig.h"
#include "HazelcastInstance.h"
#include "ILock.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientLockTest : public iTest::iTestFixture<ClientLockTest> {

            public:

                ClientLockTest(HazelcastInstanceFactory&);

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testLock();

                void testLockTtl();

                void testTryLock();

                void testForceUnlock();

                void testStats();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ILock> l;
            };

        }
    }
}


#endif
