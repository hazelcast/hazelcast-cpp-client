//
// Created by sancar koyunlu on 26/02/14.
//


#include "cluster/ClusterTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClusterTest::ClusterTest(HazelcastServerFactory &hazelcastInstanceFactory)
            :iTestFixture<ClusterTest>("ClusterTest")
            , hazelcastInstanceFactory(hazelcastInstanceFactory) {
            };


            ClusterTest::~ClusterTest() {
            }

            void ClusterTest::addTests() {
                addTest(&ClusterTest::testClusterListeners, "testClusterListeners");
                addTest(&ClusterTest::testClusterListenersFromConfig, "testClusterListenersFromConfig");
                addTest(&ClusterTest::testListenersWhenClusterDown, "testListenersWhenClusterDown");
                addTest(&ClusterTest::testBehaviourWhenClusterNotFound, "testBehaviourWhenClusterNotFound");
            };

            void ClusterTest::beforeClass() {

            };

            void ClusterTest::afterClass() {

            };

            void ClusterTest::beforeTest() {

            };

            void ClusterTest::afterTest() {

            };

            class SampleInitialListener : public InitialMembershipListener {
            public:
                SampleInitialListener(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch, util::CountDownLatch &_memberRemoved)
                :_memberAdded(_memberAdded)
                , _attributeLatch(_attributeLatch)
                , _memberRemoved(_memberRemoved) {

                };

                void init(const InitialMembershipEvent &event) {
                    std::vector<Member> const &members = event.getMembers();
                    if (members.size() == 1) {
                        _memberAdded.countDown();
                    }
                };

                void memberAdded(const MembershipEvent &event) {
                    _memberAdded.countDown();
                };

                void memberRemoved(const MembershipEvent &event) {
                    _memberRemoved.countDown();
                };


                void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                    _attributeLatch.countDown();
                };
            private:
                util::CountDownLatch &_memberAdded;
                util::CountDownLatch &_attributeLatch;
                util::CountDownLatch &_memberRemoved;
            };


            class SampleListenerInClusterTest : public MembershipListener {
            public:
                SampleListenerInClusterTest(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch, util::CountDownLatch &_memberRemoved)
                :_memberAdded(_memberAdded)
                , _attributeLatch(_attributeLatch)
                , _memberRemoved(_memberRemoved) {

                };

                void memberAdded(const MembershipEvent &event) {
                    _memberAdded.countDown();
                };

                void memberRemoved(const MembershipEvent &event) {
                    _memberRemoved.countDown();
                };

                void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                    memberAttributeEvent.getKey();
                    _attributeLatch.countDown();
                };
            private:
                util::CountDownLatch &_memberAdded;
                util::CountDownLatch &_attributeLatch;
                util::CountDownLatch &_memberRemoved;
            };

            void ClusterTest::testClusterListeners() {
                HazelcastServer instance(hazelcastInstanceFactory);
                ClientConfig clientConfig;
                HazelcastClient hazelcastClient(clientConfig.addAddress(Address(HOST, 5701)));
                Cluster cluster = hazelcastClient.getCluster();
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

                HazelcastServer instance2(hazelcastInstanceFactory);

                assertTrue(attributeLatchInit.await(1000 * 30), "attributeLatchInit");
                assertTrue(attributeLatch.await(1000 * 30), "attributeLatch");
                assertTrue(memberAdded.await(1000 * 30), "memberAdded");
                assertTrue(memberAddedInit.await(1000 * 30), "memberAddedInit");

                instance2.shutdown();

                assertTrue(memberRemoved.await(1000 * 30), "memberRemoved");
                assertTrue(memberRemovedInit.await(1000 * 30), "memberRemovedInit");

                instance.shutdown();

                cluster.removeMembershipListener(&sampleInitialListener);
                cluster.removeMembershipListener(&sampleListener);
            }

            void ClusterTest::testClusterListenersFromConfig() {
                util::CountDownLatch memberAdded(2);
                util::CountDownLatch memberAddedInit(3);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);
                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInClusterTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                ClientConfig clientConfig;
                clientConfig.addListener(&sampleListener);
                clientConfig.addListener(&sampleInitialListener);

                HazelcastServer instance(hazelcastInstanceFactory);
                HazelcastClient hazelcastClient(clientConfig.addAddress(Address(HOST, 5701)));

                HazelcastServer instance2(hazelcastInstanceFactory);

                assertTrue(attributeLatchInit.await(1000 * 30), "attributeLatchInit");
                assertTrue(attributeLatch.await(1000 * 30), "attributeLatch");
                assertTrue(memberAdded.await(1000 * 30), "memberAdded");
                assertTrue(memberAddedInit.await(1000 * 30), "memberAddedInit");

                instance2.shutdown();

                assertTrue(memberRemoved.await(1000 * 30), "memberRemoved");
                assertTrue(memberRemovedInit.await(1000 * 30), "memberRemovedInit");

                instance.shutdown();
            }

            class DummyListenerClusterTest {
            public:
                DummyListenerClusterTest(util::CountDownLatch &addLatch)
                :addLatch(addLatch) {
                };

                void entryAdded(EntryEvent<std::string, std::string> &event) {
                    addLatch.countDown();
                };

                void entryRemoved(EntryEvent<std::string, std::string> &event) {
                }

                void entryUpdated(EntryEvent<std::string, std::string> &event) {
                }

                void entryEvicted(EntryEvent<std::string, std::string> &event) {
                }

            private:
                util::CountDownLatch &addLatch;
            };

            class LclForClusterTest : public LifecycleListener {
            public:
                LclForClusterTest(util::CountDownLatch &latch)
                :latch(latch) {

                }

                void stateChanged(const LifecycleEvent &event) {
                    if (event.getState() == LifecycleEvent::CLIENT_CONNECTED) {
                        latch.countDown();
                    }
                }

            private:
                util::CountDownLatch &latch;
            };

            void ClusterTest::testListenersWhenClusterDown() {
                HazelcastServer instance(hazelcastInstanceFactory);

                ClientConfig clientConfig;
                HazelcastClient hazelcastClient(clientConfig.addAddress(Address(HOST, 5701)));

                util::CountDownLatch countDownLatch(1);
                DummyListenerClusterTest listener(countDownLatch);
                IMap <std::string, std::string> m = hazelcastClient.getMap<std::string, std::string>("testListenersWhenClusterDown");
                m.addEntryListener(listener, true);
                instance.shutdown();

                util::CountDownLatch lifecycleLatch(1);
                LclForClusterTest lifecycleListener(lifecycleLatch);
                hazelcastClient.addLifecycleListener(&lifecycleListener);

                HazelcastServer instance2(hazelcastInstanceFactory);
                lifecycleLatch.await(5000);

                m.put("sample", "entry");

                assertTrue(countDownLatch.await(10 * 1000));
                assertTrue(hazelcastClient.removeLifecycleListener(&lifecycleListener), "Listener could not removed");
            }

            void ClusterTest::testBehaviourWhenClusterNotFound() {
                ClientConfig clientConfig;
                try{
                    HazelcastClient hazelcastClient(clientConfig);
                    assertTrue(false);
                }catch(exception::IllegalStateException &e){

                }
            }
        }
    }
}


