//
// Created by sancar koyunlu on 27/02/14.
//

#include "MemberAttributeTest.h"

#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            MemberAttributeTest::MemberAttributeTest(HazelcastServerFactory &hazelcastInstanceFactory)
            :ClientTestSupport<MemberAttributeTest>("MemberAttributeTest")
            ,hazelcastInstanceFactory(hazelcastInstanceFactory){
            }


            MemberAttributeTest::~MemberAttributeTest() {
            }

            void MemberAttributeTest::addTests() {
                addTest(&MemberAttributeTest::testInitialValues, "testInitialValues");
                addTest(&MemberAttributeTest::testChangeWithListeners, "testChangeWithListeners");
            }

            void MemberAttributeTest::beforeClass() {

            }

            void MemberAttributeTest::afterClass() {

            }

            void MemberAttributeTest::beforeTest() {

            }

            void MemberAttributeTest::afterTest() {

            }

            void MemberAttributeTest::testInitialValues() {
                HazelcastServer instance(hazelcastInstanceFactory);
                ClientConfig clientConfig;
                Address address = Address(hazelcastInstanceFactory.getServerAddress(), 5701);
                HazelcastClient hazelcastClient(clientConfig.addAddress(address));
                Cluster cluster = hazelcastClient.getCluster();
                std::vector<Member> members = cluster.getMembers();
                assertEqual(1U,members.size());
                Member &member = members[0];
                assertTrue(member.lookupAttribute<int>("intAttr"));
                assertEqual(211,member.getAttribute<int>("intAttr"));

                assertTrue(member.lookupAttribute<bool>("boolAttr"));
                assertEqual(true,member.getAttribute<bool>("boolAttr"));

                assertTrue(member.lookupAttribute<byte>("byteAttr"));
                assertEqual(7,member.getAttribute<byte>("byteAttr"));

                assertTrue(member.lookupAttribute<double>("doubleAttr"));
                assertEqual(2,member.getAttribute<double>("doubleAttr"));

                assertTrue(member.lookupAttribute<float>("floatAttr"));
                assertEqual(1.2f,member.getAttribute<float>("floatAttr"));

                assertTrue(member.lookupAttribute<short>("shortAttr"));
                assertEqual(3,member.getAttribute<short>("shortAttr"));

                assertTrue(member.lookupAttribute<std::string>("strAttr"));
                assertEqual(std::string("strAttr"),member.getAttribute<std::string>("strAttr"));

                instance.shutdown();
            }


            class AttributeListener : public MembershipListener {
            public:
                AttributeListener(util::CountDownLatch &_attributeLatch)
                  : _attributeLatch(_attributeLatch){

                }

                void memberAdded(const MembershipEvent &event) {
                }

                void memberRemoved(const MembershipEvent &event) {
                }

                void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                    if(memberAttributeEvent.getOperationType() != MemberAttributeEvent::PUT){
                        return;
                    }
                    const std::string &key = memberAttributeEvent.getKey();
                    if(key == "intAttr"){
                        boost::shared_ptr<int> ptr = memberAttributeEvent.getValue<int>();
                        if(ptr.get() != NULL && 211 == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "boolAttr"){
                        boost::shared_ptr<bool> ptr = memberAttributeEvent.getValue<bool>();
                        if(ptr.get() != NULL && true == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "byteAttr"){
                        boost::shared_ptr<byte> ptr = memberAttributeEvent.getValue<byte>();
                        if(ptr.get() != NULL && 7 == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "doubleAttr"){
                        boost::shared_ptr<double> ptr = memberAttributeEvent.getValue<double>();
                        if(ptr.get() != NULL && 2 == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "floatAttr"){
                        boost::shared_ptr<float> ptr = memberAttributeEvent.getValue<float>();
                        if(ptr.get() != NULL && 1.2f == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "shortAttr"){
                        boost::shared_ptr<short> ptr = memberAttributeEvent.getValue<short>();
                        if(ptr.get() != NULL && 3 == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "strAttr"){
                        boost::shared_ptr<std::string> ptr = memberAttributeEvent.getValue<std::string>();
                        if(ptr.get() != NULL && std::string("strAttr") == *ptr ){
                            _attributeLatch.countDown();
                        }
                    }
                }
            private:
                util::CountDownLatch &_attributeLatch;
            };

            void MemberAttributeTest::testChangeWithListeners(){
                util::CountDownLatch attributeLatch(7);
                AttributeListener sampleListener(attributeLatch);

                ClientConfig clientConfig;
                clientConfig.addListener(&sampleListener);

                HazelcastServer instance(hazelcastInstanceFactory);
                Address address = Address(hazelcastInstanceFactory.getServerAddress(), 5701);
                HazelcastClient hazelcastClient(clientConfig.addAddress(address));

                HazelcastServer instance2(hazelcastInstanceFactory);

                bool a = attributeLatch.await(30);
                assertTrue(a, "attributeLatch");

                instance2.shutdown();

                instance.shutdown();
            }

        }
    }
}

