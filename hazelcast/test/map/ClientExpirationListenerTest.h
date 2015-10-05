//
// Created by sancar koyunlu on 02/10/15.
//


#ifndef HAZELCAST_ClientExpirationListenerTest
#define HAZELCAST_ClientExpirationListenerTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientExpirationListenerTest : public ClientTestSupport<ClientExpirationListenerTest> {
            public:
                ClientExpirationListenerTest(HazelcastServerFactory &);

                ~ClientExpirationListenerTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void notified_afterExpirationOfEntries();

                void bothNotified_afterExpirationOfEntries();

            private:
                HazelcastServer instance;
                HazelcastServer instance2;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IMap<int, int > > imap;
            };
        }
    }
}



#endif //HAZELCAST_ClientExpirationListenerTest
