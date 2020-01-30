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
#include <gtest/gtest.h>

#include "HazelcastServer.h"
#include <ClientTestSupport.h>
#include "ClientTestSupportBase.h"
#include "HazelcastServerFactory.h"

#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class SimpleListenerTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig *> {
            public:
                SimpleListenerTest() {}

            protected:
                class MyEntryListener : public EntryListener<int, int> {
                public:
                    MyEntryListener(util::CountDownLatch &mapClearedLatch) : mapClearedLatch(mapClearedLatch) {}

                    virtual void entryAdded(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryRemoved(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryUpdated(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryEvicted(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryExpired(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryMerged(const EntryEvent<int, int> &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                        assertEquals("testDeregisterListener", event.getName());
                        assertEquals(EntryEventType::CLEAR_ALL, event.getEventType());
                        std::string hostName = event.getMember().getAddress().getHost();
                        assertTrue(hostName == "127.0.0.1" || hostName == "localhost");
                        assertEquals(5701, event.getMember().getAddress().getPort());
                        assertEquals(1, event.getNumberOfEntriesAffected());
                        std::cout << "Map cleared event received:" << event << std::endl;
                        mapClearedLatch.countDown();
                    }

                private:
                    util::CountDownLatch &mapClearedLatch;
                };
                
                class SampleInitialListener : public InitialMembershipListener {
                public:
                    SampleInitialListener(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch,
                                          util::CountDownLatch &_memberRemoved)
                            : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch),
                              _memberRemoved(_memberRemoved) {

                    }

                    void init(const InitialMembershipEvent &event) {
                        std::vector<Member> const &members = event.getMembers();
                        if (members.size() == 1) {
                            _memberAdded.countDown();
                        }
                    }

                    void memberAdded(const MembershipEvent &event) {
                        _memberAdded.countDown();
                    }

                    void memberRemoved(const MembershipEvent &event) {
                        _memberRemoved.countDown();
                    }


                    void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                        _attributeLatch.countDown();
                    }

                private:
                    util::CountDownLatch &_memberAdded;
                    util::CountDownLatch &_attributeLatch;
                    util::CountDownLatch &_memberRemoved;
                };

                class SampleListenerInSimpleListenerTest : public MembershipListener {
                public:
                    SampleListenerInSimpleListenerTest(util::CountDownLatch &_memberAdded,
                                                util::CountDownLatch &_attributeLatch,
                                                util::CountDownLatch &_memberRemoved)
                            : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch),
                              _memberRemoved(_memberRemoved) {

                    }

                    void memberAdded(const MembershipEvent &event) {
                        _memberAdded.countDown();
                    }

                    void memberRemoved(const MembershipEvent &event) {
                        _memberRemoved.countDown();
                    }

                    void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                        memberAttributeEvent.getKey();
                        _attributeLatch.countDown();
                    }

                private:
                    util::CountDownLatch &_memberAdded;
                    util::CountDownLatch &_attributeLatch;
                    util::CountDownLatch &_memberRemoved;
                };
            };

            class SmartTcpClientConfig : public ClientConfig {
            };

            class NonSmartTcpClientConfig : public ClientConfig {
            public:
                NonSmartTcpClientConfig() {
                    getNetworkConfig().setSmartRouting(false);
                }
            };

            TEST_P(SimpleListenerTest, testSharedClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*const_cast<ParamType &>(GetParam()));
                Cluster cluster = hazelcastClient.getCluster();
                util::CountDownLatch memberAdded(1);
                util::CountDownLatch memberAddedInit(2);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);

                std::shared_ptr<MembershipListener> sampleInitialListener(
                        new SampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit));
                std::shared_ptr<MembershipListener> sampleListener(
                        new SampleListenerInSimpleListenerTest(memberAdded, attributeLatch, memberRemoved));

                std::string initialListenerRegistrationId = cluster.addMembershipListener(sampleInitialListener);
                std::string sampleListenerRegistrationId = cluster.addMembershipListener(sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_TRUE(memberAdded.await(30));
                ASSERT_TRUE(memberAddedInit.await(30));

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_TRUE(attributeLatchInit.await(30));
                ASSERT_TRUE(attributeLatch.await(30));

                instance2.shutdown();

                ASSERT_TRUE(memberRemoved.await(30));
                ASSERT_TRUE(memberRemovedInit.await(30));

                instance.shutdown();

                ASSERT_TRUE(cluster.removeMembershipListener(initialListenerRegistrationId));
                ASSERT_TRUE(cluster.removeMembershipListener(sampleListenerRegistrationId));
            }

            TEST_P(SimpleListenerTest, testClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*const_cast<ParamType &>(GetParam()));
                Cluster cluster = hazelcastClient.getCluster();
                util::CountDownLatch memberAdded(1);
                util::CountDownLatch memberAddedInit(2);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);

                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                cluster.addMembershipListener(&sampleInitialListener);
                cluster.addMembershipListener(&sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_TRUE(memberAdded.await(30));
                ASSERT_TRUE(memberAddedInit.await(30));

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_OPEN_EVENTUALLY(attributeLatchInit);
                ASSERT_OPEN_EVENTUALLY(attributeLatch);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();

                ASSERT_TRUE(cluster.removeMembershipListener(&sampleInitialListener));
                ASSERT_TRUE(cluster.removeMembershipListener(&sampleListener));
            }

            TEST_P(SimpleListenerTest, testClusterListenersFromConfig) {
                util::CountDownLatch memberAdded(1);
                util::CountDownLatch memberAddedInit(1);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);
                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.addListener(&sampleListener);
                clientConfig.addListener(&sampleInitialListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(clientConfig);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_OPEN_EVENTUALLY(attributeLatchInit);
                ASSERT_OPEN_EVENTUALLY(attributeLatch);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();
            }

            TEST_P(SimpleListenerTest, testDeregisterListener) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                HazelcastClient hazelcastClient(clientConfig);

                IMap<int, int> map = hazelcastClient.getMap<int, int>("testDeregisterListener");

                ASSERT_FALSE(map.removeEntryListener("Unknown"));

                util::CountDownLatch mapClearedLatch(1);
                MyEntryListener listener(mapClearedLatch);
                std::string listenerRegistrationId = map.addEntryListener(listener, true);

                map.put(1, 1);

                map.clear();

                assertOpenEventually(mapClearedLatch);

                assertTrue(map.removeEntryListener(listenerRegistrationId));
            }

            INSTANTIATE_TEST_CASE_P(All,
                                    SimpleListenerTest,
                                    ::testing::Values(new SmartTcpClientConfig(), new NonSmartTcpClientConfig()));
        }
    }
}


