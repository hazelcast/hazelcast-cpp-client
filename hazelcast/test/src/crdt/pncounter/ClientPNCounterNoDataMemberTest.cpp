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

namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class ClientPNCounterNoDataMemberTest : public ClientTestSupport {
                    };

                    TEST_F(ClientPNCounterNoDataMemberTest, noDataMemberExceptionIsThrown) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-lite-member.xml");
                        HazelcastServer instance(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_THROW(pnCounter->addAndGet(5), exception::NoDataMemberInClusterException);
                    }

                }
            }

        }
    }
}
