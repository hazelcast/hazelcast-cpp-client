//
//  IdGeneratorTest.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_IdGeneratorTest_h
#define hazelcast_IdGeneratorTest_h


#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/IdGenerator.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API IdGeneratorTest : public iTest::iTestFixture<IdGeneratorTest> {

            public:
                IdGeneratorTest(HazelcastServerFactory &);

                ~IdGeneratorTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testGenerator();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IdGenerator> generator;

            };
        }
    }
}

#endif