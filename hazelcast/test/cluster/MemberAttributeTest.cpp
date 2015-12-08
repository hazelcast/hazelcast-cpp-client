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
#include "hazelcast/client/MemberAttributeEvent.h"

#include <memory>

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            MemberAttributeTest::MemberAttributeTest(HazelcastServerFactory &hazelcastInstanceFactory)
            :ClientTestSupport<MemberAttributeTest>("MemberAttributeTest" , &hazelcastInstanceFactory)
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
                std::auto_ptr<HazelcastClient> hazelcastClient(getNewClient());
                Cluster cluster = hazelcastClient->getCluster();
                std::vector<Member> members = cluster.getMembers();
                assertEqual(1U,members.size());
                Member &member = members[0];
                assertTrue(member.lookupAttribute("intAttr"));
                assertEqual("211", *member.getAttribute("intAttr"));

                assertTrue(member.lookupAttribute("boolAttr"));
                assertEqual("true", *member.getAttribute("boolAttr"));

                assertTrue(member.lookupAttribute("byteAttr"));
                assertEqual("7", *member.getAttribute("byteAttr"));

                assertTrue(member.lookupAttribute("doubleAttr"));
                assertEqual("2.0", *member.getAttribute("doubleAttr"));

                assertTrue(member.lookupAttribute("floatAttr"));
                assertEqual("1.2", *member.getAttribute("floatAttr"));

                assertTrue(member.lookupAttribute("shortAttr"));
                assertEqual("3", *member.getAttribute("shortAttr"));

                assertTrue(member.lookupAttribute("strAttr"));
                assertEqual(std::string("strAttr"), *member.getAttribute("strAttr"));

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
                        const std::string &value = memberAttributeEvent.getValue();
                        if("211" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "boolAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if("true" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "byteAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if("7" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "doubleAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if("2.0" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "floatAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if("1.2" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "shortAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if("3" == value ){
                            _attributeLatch.countDown();
                        }
                    }else if(key == "strAttr"){
                        const std::string &value = memberAttributeEvent.getValue();
                        if(std::string("strAttr") == value ){
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

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->addListener(&sampleListener);

                HazelcastServer instance(hazelcastInstanceFactory);
                HazelcastClient hazelcastClient(*clientConfig);

                HazelcastServer instance2(hazelcastInstanceFactory);

                bool a = attributeLatch.await(30);
                assertTrue(a, "attributeLatch");

                instance2.shutdown();

                instance.shutdown();
            }

        }
    }
}

