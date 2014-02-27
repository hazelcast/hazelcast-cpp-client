//
// Created by sancar koyunlu on 26/02/14.
//


#include "ClusterTest.h"
#include "HazelcastInstanceFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClusterTest::ClusterTest(HazelcastInstanceFactory &hazelcastInstanceFactory)
            :iTestFixture("ClusterTest")
            ,hazelcastInstanceFactory(hazelcastInstanceFactory){
            };


            ClusterTest::~ClusterTest() {
            }

            void ClusterTest::addTests() {
                addTest(&ClusterTest::testClusterListeners, "testClusterListeners");
                addTest(&ClusterTest::testClusterListenersFromConfig, "testClusterListenersFromConfig");
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


            class Samplelistener : public MembershipListener {
            public:
                Samplelistener(util::CountDownLatch &_memberAdded, util::CountDownLatch &_attributeLatch, util::CountDownLatch &_memberRemoved)
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
                HazelcastInstance instance(hazelcastInstanceFactory);
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
                Samplelistener sampleListener(memberAdded, attributeLatch, memberRemoved);

                cluster.addMembershipListener(&sampleInitialListener);
                cluster.addMembershipListener(&sampleListener);

                HazelcastInstance instance2(hazelcastInstanceFactory);

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

            void ClusterTest::testClusterListenersFromConfig(){
                util::CountDownLatch memberAdded(2);
                util::CountDownLatch memberAddedInit(3);
                util::CountDownLatch memberRemoved(1);
                util::CountDownLatch memberRemovedInit(1);
                util::CountDownLatch attributeLatch(7);
                util::CountDownLatch attributeLatchInit(7);
                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                Samplelistener sampleListener(memberAdded, attributeLatch, memberRemoved);

                ClientConfig clientConfig;
                clientConfig.addListener(&sampleListener);
                clientConfig.addListener(&sampleInitialListener);

                HazelcastInstance instance(hazelcastInstanceFactory);
                HazelcastClient hazelcastClient(clientConfig.addAddress(Address(HOST, 5701)));

                HazelcastInstance instance2(hazelcastInstanceFactory);

                assertTrue(attributeLatchInit.await(1000 * 30), "attributeLatchInit");
                assertTrue(attributeLatch.await(1000 * 30), "attributeLatch");
                assertTrue(memberAdded.await(1000 * 30), "memberAdded");
                assertTrue(memberAddedInit.await(1000 * 30), "memberAddedInit");

                instance2.shutdown();

                assertTrue(memberRemoved.await(1000 * 30), "memberRemoved");
                assertTrue(memberRemovedInit.await(1000 * 30), "memberRemovedInit");

                instance.shutdown();



            }

        }
    }
}

