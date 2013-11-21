//
//  ClientTopicTest.h
//  hazelcast
//
//  Created by Sancar on 19.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientTopicTest_h
#define hazelcast_ClientTopicTest_h

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ITopic.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientTopicTest : public iTest::iTestFixture<ClientTopicTest> {

            public:

                ClientTopicTest(HazelcastInstanceFactory&);

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testTopicListeners();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr< ITopic<std::string> > topic;
            };

        }
    }
}


#endif
