/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExpirationListenerTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    // clear maps
                    try {
                        imap->clear();
                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "[TearDown] An exception occured in tear down:" << e.what();
                        util::ILogger::getLogger().warning(out.str());
                    }
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    clientConfig->getProperties()[ClientProperties::PROP_HEARTBEAT_TIMEOUT] = "20";
                    client = new HazelcastClient(*clientConfig);
                    imap = new IMap<int, int>(client->getMap<int, int>("IntMap"));
                }

                static void TearDownTestCase() {
                    delete imap;
                    delete client;
                    delete clientConfig;
                    delete instance2;
                    delete instance;

                    imap = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance2 = NULL;
                    instance = NULL;
                }
                
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static IMap<int, int> *imap;
            };

            HazelcastServer *ClientExpirationListenerTest::instance = NULL;
            HazelcastServer *ClientExpirationListenerTest::instance2 = NULL;
            ClientConfig *ClientExpirationListenerTest::clientConfig = NULL;
            HazelcastClient *ClientExpirationListenerTest::client = NULL;
            IMap<int, int> *ClientExpirationListenerTest::imap = NULL;

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
                int numberOfPutOperations = 10000;
                util::CountDownLatch expirationEventArrivalCount(numberOfPutOperations);

                ExpirationListener expirationListener(expirationEventArrivalCount);
                std::string registrationId = imap->addEntryListener(expirationListener, true);

                for (int i = 0; i < numberOfPutOperations; i++) {
                    time_t start = time(NULL);
                    imap->put(i, i, 100);
                    time_t diff = time(NULL) - start;
                    if (diff > 1) { // more than 1 sec
                        char msg[200];
                        util::hz_snprintf(msg, 200, "[notified_afterExpirationOfEntries] put for %d took %lld secs", i,
                                          diff);
                        util::ILogger::getLogger().warning(msg);
                    }
                }

                // wait expiration of entries.
                hazelcast::util::sleep(1);

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; ++i) {
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
