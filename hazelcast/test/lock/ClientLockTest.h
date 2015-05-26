//
//  ClientLockTest.h
//  hazelcast
//
//  Created by Sancar on 15.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientLockTest_h
#define hazelcast_ClientLockTest_h

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/ILock.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientLockTest : public ClientTestSupport<ClientLockTest> {

            public:

                ClientLockTest(HazelcastServerFactory &);

                ~ClientLockTest();

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
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ILock> l;
            };

        }
    }
}


#endif

