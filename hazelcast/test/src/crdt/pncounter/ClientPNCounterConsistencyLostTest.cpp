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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
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
                        boost::shared_ptr<Address> getCurrentTargetReplicaAddress(
                                const boost::shared_ptr<client::crdt::pncounter::PNCounter> &pnCounter) {
                            return boost::static_pointer_cast<proxy::ClientPNCounterProxy>(pnCounter)->getCurrentTargetReplicaAddress();
                        }

                        void terminateMember(const Address &address, HazelcastServer &server1, HazelcastServer &server2) {
                            if (address == Address(server1.getMember().getAddress())) {
                                server1.terminate();
                                return;
                            }

                            if (address == Address(server2.getMember().getAddress())) {
                                server2.terminate();
                                return;
                            }
                        }
                    };

                    TEST_F(ClientPNCounterConsistencyLostTest, consistencyLostExceptionIsThrownWhenTargetReplicaDisappears) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        ClientConfig clientConfig;
                        HazelcastClient client(clientConfig);

                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                        assertEquals(5, pnCounter->get());

                        boost::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        ASSERT_THROW(pnCounter->addAndGet(5), exception::ConsistencyLostException);
                    }

                    TEST_F(ClientPNCounterConsistencyLostTest, driverCanContinueSessionByCallingReset) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        ClientConfig clientConfig;
                        HazelcastClient client(clientConfig);

                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                        assertEquals(5, pnCounter->get());

                        boost::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        pnCounter->reset();

                        pnCounter->addAndGet(5);
                    }
                }
            }

        }
    }
}
