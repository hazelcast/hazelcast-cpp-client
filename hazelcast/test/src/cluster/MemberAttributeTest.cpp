/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include <ClientTestSupport.h>

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class MemberAttributeTest : public ClientTestSupport
            {};

            TEST_F(MemberAttributeTest, testInitialValues) {
                HazelcastServer instance(*g_srvFactory);
                std::auto_ptr<HazelcastClient> hazelcastClient(getNewClient());
                Cluster cluster = hazelcastClient->getCluster();
                std::vector<Member> members = cluster.getMembers();
                ASSERT_EQ(1U,members.size());
                Member &member = members[0];
                ASSERT_TRUE(member.lookupAttribute("intAttr"));
                ASSERT_EQ("211", *member.getAttribute("intAttr"));

                ASSERT_TRUE(member.lookupAttribute("boolAttr"));
                ASSERT_EQ("true", *member.getAttribute("boolAttr"));

                ASSERT_TRUE(member.lookupAttribute("byteAttr"));
                ASSERT_EQ("7", *member.getAttribute("byteAttr"));

                ASSERT_TRUE(member.lookupAttribute("doubleAttr"));
                ASSERT_EQ("2.0", *member.getAttribute("doubleAttr"));

                ASSERT_TRUE(member.lookupAttribute("floatAttr"));
                ASSERT_EQ("1.2", *member.getAttribute("floatAttr"));

                ASSERT_TRUE(member.lookupAttribute("shortAttr"));
                ASSERT_EQ("3", *member.getAttribute("shortAttr"));

                ASSERT_TRUE(member.lookupAttribute("strAttr"));
                ASSERT_EQ(std::string("strAttr"), *member.getAttribute("strAttr"));

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

            TEST_F(MemberAttributeTest, testChangeWithListeners){
                util::CountDownLatch attributeLatch(7);
                AttributeListener sampleListener(attributeLatch);

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->addListener(&sampleListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*clientConfig);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_TRUE(attributeLatch.await(30));

                instance2.shutdown();

                instance.shutdown();
            }

        }
    }
}

