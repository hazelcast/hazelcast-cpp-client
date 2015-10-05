//
// Created by sancar koyunlu on 02/10/15.
//

#include "map/ClientExpirationListenerTest.h"
#include "hazelcast/client/EntryAdapter.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;


            ClientExpirationListenerTest::ClientExpirationListenerTest(HazelcastServerFactory& serverFactory)
            : ClientTestSupport<ClientExpirationListenerTest>("ClientExpirationListenerTest", &serverFactory)
            , instance(serverFactory)
            , instance2(serverFactory)
            , client(getNewClient())
            , imap(new IMap<int, int>(client->getMap<int, int>("clientExpirationListenerTest"))) {

            }

            ClientExpirationListenerTest::~ClientExpirationListenerTest() {

            }

            void ClientExpirationListenerTest::addTests() {
                addTest(&ClientExpirationListenerTest::notified_afterExpirationOfEntries, "notified_afterExpirationOfEntries");
                addTest(&ClientExpirationListenerTest::bothNotified_afterExpirationOfEntries, "bothNotified_afterExpirationOfEntries");
            }

            void ClientExpirationListenerTest::beforeClass() {

            }

            void ClientExpirationListenerTest::afterClass() {

            }

            void ClientExpirationListenerTest::beforeTest() {

            }

            void ClientExpirationListenerTest::afterTest() {

            }

            class ExpirationListener : public EntryAdapter<int, int> {
            public:
                ExpirationListener(util::CountDownLatch& latch)
                : latch(latch) {

                }

                void entryExpired(const EntryEvent<int, int>& event) {
                    latch.countDown();
                }

            private:
                util::CountDownLatch& latch;
            };

            void ClientExpirationListenerTest::notified_afterExpirationOfEntries() {
                int numberOfPutOperations = 1000;
                util::CountDownLatch expirationEventArrivalCount(numberOfPutOperations);

                ExpirationListener expirationListener(expirationEventArrivalCount);
                std::string registrationId = imap->addEntryListener(expirationListener, true);

                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->put(i, i, 100);
                }

                // wait expiration of entries.
                hazelcast::util::sleep(1);

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->get(i);
                }

                assertTrue(expirationEventArrivalCount.await(120));
                assertTrue(imap->removeEntryListener(registrationId));
            }


            class ExpirationAndEvictionListener : public EntryAdapter<int, int> {
            public:
                ExpirationAndEvictionListener(util::CountDownLatch& evictedLatch, util::CountDownLatch& expiredLatch)
                : evictedLatch(evictedLatch)
                , expiredLatch(expiredLatch){

                }

                void entryEvicted(const EntryEvent<int, int>& event) {
                    evictedLatch.countDown();
                }

                void entryExpired(const EntryEvent<int, int>& event) {
                    expiredLatch.countDown();
                }

            private:
                util::CountDownLatch& evictedLatch;
                util::CountDownLatch& expiredLatch;
            };

            void ClientExpirationListenerTest::bothNotified_afterExpirationOfEntries() {
                int numberOfPutOperations = 1000;
                util::CountDownLatch expirationEventArrivalCount(numberOfPutOperations);
                util::CountDownLatch evictedEventArrivalCount(numberOfPutOperations);

                ExpirationAndEvictionListener expirationListener(expirationEventArrivalCount, evictedEventArrivalCount);
                std::string registrationId = imap->addEntryListener(expirationListener, true);

                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->put(i, i, 100);
                }

                // wait expiration of entries.
                hazelcast::util::sleep(1);

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->get(i);
                }

                assertTrue(expirationEventArrivalCount.await(120));
                assertTrue(evictedEventArrivalCount.await(120));
                assertTrue(imap->removeEntryListener(registrationId));
            }


        }
    }
}
