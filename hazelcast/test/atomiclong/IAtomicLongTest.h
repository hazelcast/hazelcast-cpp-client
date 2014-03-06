//
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientAtomicLong_h
#define hazelcast_ClientAtomicLong_h

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "HazelcastServer.h"
#include "hazelcast/client/IAtomicLong.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API IAtomicLongTest : public iTest::iTestFixture<IAtomicLongTest> {

            public:

                IAtomicLongTest(HazelcastServerFactory &);

                ~IAtomicLongTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void test();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IAtomicLong > atom;
            };
        }
    }
}
#endif
