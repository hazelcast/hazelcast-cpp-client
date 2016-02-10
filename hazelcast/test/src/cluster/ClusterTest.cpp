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
// Created by sancar koyunlu on 26/02/14.
//

#include "ClientTestSupport.h"
#include <hazelcast/util/CountDownLatch.h>
#include <hazelcast/client/MembershipListener.h>
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServer.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClusterTest : public ClientTestSupport {
            };

            class SampleInitialListener : public InitialMembershipListener {
            public:
                SampleInitialListener(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch,
                                      util::CountDownLatch &_memberRemoved)
                        : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch), _memberRemoved(_memberRemoved) {

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


            class SampleListenerInClusterTest : public MembershipListener {
            public:
                SampleListenerInClusterTest(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch,
                                            util::CountDownLatch &_memberRemoved)
                        : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch), _memberRemoved(_memberRemoved) {

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

            TEST_F(ClusterTest, testClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                std::auto_ptr<HazelcastClient> hazelcastClient(getNewClient());
                Cluster cluster = hazelcastClient->getCluster();
                util::CountDownLatch memberAdded(1);
                util::CountDownLatch memberAddedInit(2);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);

                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInClusterTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                cluster.addMembershipListener(&sampleInitialListener);
                cluster.addMembershipListener(&sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_TRUE(attributeLatchInit.await(30));
                ASSERT_TRUE(attributeLatch.await(30));
                ASSERT_TRUE(memberAdded.await(30));
                ASSERT_TRUE(memberAddedInit.await(30));

                instance2.shutdown();

                ASSERT_TRUE(memberRemoved.await(30));
                ASSERT_TRUE(memberRemovedInit.await(30));

                instance.shutdown();

                cluster.removeMembershipListener(&sampleInitialListener);
                cluster.removeMembershipListener(&sampleListener);
            }

            TEST_F(ClusterTest, testClusterListenersFromConfig) {
                util::CountDownLatch memberAdded(2);
                util::CountDownLatch memberAddedInit(3);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);
                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInClusterTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->addListener(&sampleListener);
                clientConfig->addListener(&sampleInitialListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*clientConfig);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_TRUE(attributeLatchInit.await(30));
                ASSERT_TRUE(attributeLatch.await(30));
                ASSERT_TRUE(memberAdded.await(30));
                ASSERT_TRUE(memberAddedInit.await(30));

                instance2.shutdown();

                ASSERT_TRUE(memberRemoved.await(30));
                ASSERT_TRUE(memberRemovedInit.await(30));

                instance.shutdown();
            }

            class DummyListenerClusterTest : public EntryAdapter<std::string, std::string> {
            public:
                DummyListenerClusterTest(util::CountDownLatch &addLatch)
                        : addLatch(addLatch) {
                }

                void entryAdded(const EntryEvent<std::string, std::string> &event) {
                    addLatch.countDown();
                }

            private:
                util::CountDownLatch &addLatch;
            };

            class LclForClusterTest : public LifecycleListener {
            public:
                LclForClusterTest(util::CountDownLatch &latch)
                        : latch(latch) {

                }

                void stateChanged(const LifecycleEvent &event) {
                    if (event.getState() == LifecycleEvent::CLIENT_CONNECTED) {
                        latch.countDown();
                    }
                }

            private:
                util::CountDownLatch &latch;
            };

            TEST_F(ClusterTest, testListenersWhenClusterDown) {
                HazelcastServer instance(*g_srvFactory);

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->setAttemptPeriod(1000 * 10).setConnectionAttemptLimit(100).setLogLevel(FINEST);
                HazelcastClient hazelcastClient(*clientConfig);

                util::CountDownLatch countDownLatch(1);
                DummyListenerClusterTest listener(countDownLatch);
                IMap<std::string, std::string> m = hazelcastClient.getMap<std::string, std::string>(
                        "testListenersWhenClusterDown");
                m.addEntryListener(listener, true);
                instance.shutdown();

                util::CountDownLatch lifecycleLatch(1);
                LclForClusterTest lifecycleListener(lifecycleLatch);
                hazelcastClient.addLifecycleListener(&lifecycleListener);

                HazelcastServer instance2(*g_srvFactory);
                ASSERT_TRUE(lifecycleLatch.await(120));
                // Let enough time for the client to re-register the failed listeners
                util::sleep(1);
                m.put("sample", "entry");
                ASSERT_TRUE(countDownLatch.await(60));
                ASSERT_TRUE(hazelcastClient.removeLifecycleListener(&lifecycleListener));
            }

            TEST_F(ClusterTest, testBehaviourWhenClusterNotFound) {
                ClientConfig clientConfig;
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }
        }
    }
}


