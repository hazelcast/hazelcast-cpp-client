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
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/proxy/ClientPNCounterProxy.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    /**
                     * Client implementation for testing behaviour of {@link ConsistencyLostException}
                     */
                    class ClientPNCounterConsistencyLostTest : public ClientTestSupport {
                    protected:
                        std::shared_ptr<Address> getCurrentTargetReplicaAddress(
                                const std::shared_ptr<client::crdt::pncounter::PNCounter> &pnCounter) {
                            return std::static_pointer_cast<proxy::ClientPNCounterProxy>(pnCounter)->getCurrentTargetReplicaAddress();
                        }

                        void terminateMember(const Address &address, HazelcastServer &server1, HazelcastServer &server2) {
                            auto member1 = server1.getMember();
                            if (address == Address(member1.host, member1.port)) {
                                server1.terminate();
                                return;
                            }

                            auto member2 = server2.getMember();
                            if (address == Address(member2.host, member2.port)) {
                                server2.terminate();
                                return;
                            }
                        }
                    };

                    TEST_F(ClientPNCounterConsistencyLostTest, consistencyLostExceptionIsThrownWhenTargetReplicaDisappears) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                        assertEquals(5, pnCounter->get());

                        std::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        ASSERT_THROW(pnCounter->addAndGet(5), exception::ConsistencyLostException);
                    }

                    TEST_F(ClientPNCounterConsistencyLostTest, driverCanContinueSessionByCallingReset) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                        assertEquals(5, pnCounter->get());

                        std::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        pnCounter->reset();

                        pnCounter->addAndGet(5);
                    }
                }
            }

        }
    }
}
