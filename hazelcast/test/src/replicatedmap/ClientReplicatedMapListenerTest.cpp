/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/query/FalsePredicate.h>
#include "HazelcastServerFactory.h"

#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapListenerTest : public ClientTestSupport {
            protected:
                class EventCountingListener : public EntryListener<int, int> {
                public:
                    EventCountingListener() : keys(UINT_MAX) {}

                    virtual void entryAdded(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++addCount;
                    }

                    virtual void entryRemoved(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++removeCount;
                    }

                    virtual void entryUpdated(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++updateCount;
                    }

                    virtual void entryEvicted(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++evictCount;
                    }

                    virtual void entryExpired(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryMerged(const EntryEvent<int, int> &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                        ++mapEvictCount;
                    }

                    virtual void mapCleared(const MapEvent &event) {
                        ++mapClearCount;
                    }

                    util::BlockingConcurrentQueue<int> keys;
                    util::AtomicInt addCount;
                    util::AtomicInt removeCount;
                    util::AtomicInt updateCount;
                    util::AtomicInt evictCount;
                    util::AtomicInt mapClearCount;
                    util::AtomicInt mapEvictCount;
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance1 = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    client2 = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete instance1;
                    delete instance2;

                    client = NULL;
                    client2 = NULL;
                    instance1 = NULL;
                    instance2 = NULL;
                }

                static HazelcastServer *instance1;
                static HazelcastServer *instance2;
                static HazelcastClient *client;
                static HazelcastClient *client2;
            };

            HazelcastServer *ClientReplicatedMapListenerTest::instance1 = NULL;
            HazelcastServer *ClientReplicatedMapListenerTest::instance2 = NULL;
            HazelcastClient *ClientReplicatedMapListenerTest::client = NULL;
            HazelcastClient *ClientReplicatedMapListenerTest::client2 = NULL;

            TEST_F(ClientReplicatedMapListenerTest, testEntryAdded) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                ASSERT_EQ_EVENTUALLY(1, listener->addCount.get());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryUpdated) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->put(1, 2);
                ASSERT_EQ_EVENTUALLY(1, listener->updateCount.get());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryRemoved) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->remove(1);
                ASSERT_EQ_EVENTUALLY(1, listener->removeCount.get());
            }

            TEST_F(ClientReplicatedMapListenerTest, testMapClear) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->clear();
                ASSERT_EQ_EVENTUALLY(1, listener->mapClearCount.get());
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenToKeyForEntryAdded) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener, 1);
                replicatedMap->put(1, 1);
                replicatedMap->put(2, 2);
                ASSERT_TRUE_EVENTUALLY(listener->keys.size() == 1U && listener->keys.pop() == 1 && listener->addCount.get() == 1);
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenWithPredicate) {
                boost::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(getTestName());
                boost::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener, query::FalsePredicate());
                replicatedMap->put(2, 2);
                // Check for 3 seconds
                ASSERT_TRUE_ALL_THE_TIME((listener->addCount.get() == 0), 3);
            }

        }
    }
}
