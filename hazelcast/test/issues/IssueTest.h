//
// Created by sancar koyunlu on 21/04/14.
//


#ifndef HAZELCAST_IssueTest
#define HAZELCAST_IssueTest

#include "iTest/iTest.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class IssueTest : public iTest::iTestFixture<IssueTest> {

            public:
                IssueTest(HazelcastServerFactory &);

                ~IssueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testOperationRedo_smartRoutingDisabled();

                /**
                 * This test tests the changes for customer reported zendesk issue 864.
                 */
                void testListenerSubscriptionOnSingleServerRestart();

            private:
                class Issue864MapListener : public hazelcast::client::EntryListener<int, int> {

                public:

                    Issue864MapListener(util::CountDownLatch &l);

                    virtual void entryAdded(const EntryEvent<int, int> &event);

                    virtual void entryRemoved(const EntryEvent<int, int> &event);

                    virtual void entryUpdated(const EntryEvent<int, int> &event);

                    virtual void entryEvicted(const EntryEvent<int, int> &event);

                    virtual void mapEvicted(const MapEvent &event);

                    virtual void mapCleared(const MapEvent &event);
                private:
                    util::CountDownLatch &latch;
                };

                HazelcastServerFactory& serverFactory;
                util::CountDownLatch latch;
                Issue864MapListener listener;
            };
        }
    }
}


#endif //HAZELCAST_IssueTest
