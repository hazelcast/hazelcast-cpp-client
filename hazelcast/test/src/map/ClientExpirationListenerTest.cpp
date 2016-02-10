/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 02/10/15.
//

#include "hazelcast/util/Util.h"
#include "map/ClientExpirationListenerTest.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ClientExpirationListenerTest::ClientExpirationListenerTest()
            : instance(*g_srvFactory)
            , instance2(*g_srvFactory)
            , client(getNewClient())
            , imap(new IMap<int, int>(client->getMap<int, int>("clientExpirationListenerTest"))) {
            }

            ClientExpirationListenerTest::~ClientExpirationListenerTest() {
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

            TEST_F(ClientExpirationListenerTest, notified_afterExpirationOfEntries) {
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

                ASSERT_TRUE(expirationEventArrivalCount.await(120));
                ASSERT_TRUE(imap->removeEntryListener(registrationId));
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

            TEST_F(ClientExpirationListenerTest, bothNotified_afterExpirationOfEntries) {
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

                ASSERT_TRUE(expirationEventArrivalCount.await(120));
                ASSERT_TRUE(evictedEventArrivalCount.await(120));
                ASSERT_TRUE(imap->removeEntryListener(registrationId));
            }
        }
    }
}
