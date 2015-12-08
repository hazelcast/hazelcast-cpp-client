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
// Created by İhsan Demir on 26/05/15.
//

#ifndef HAZELCASTCLIENT_CLIENTTESTSUPPORT_H
#define HAZELCASTCLIENT_CLIENTTESTSUPPORT_H

#include <memory>

#include "iTest/iTestFixture.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServerFactory.h"
#include "iTest/iTest.h"

namespace hazelcast {
    namespace client {
        namespace test {
            template <class T>
            class ClientTestSupport : public iTest::iTestFixture<T> {
            public:
                ClientTestSupport(const std::string &fixtureName, const HazelcastServerFactory *factory = NULL) :
                iTest::iTestFixture<T>(fixtureName), serverFactory(factory) {
                }

                std::auto_ptr<hazelcast::client::ClientConfig> getConfig() {
                    std::auto_ptr<hazelcast::client::ClientConfig> clientConfig(new ClientConfig());
                    if (NULL != serverFactory) {
                        clientConfig->addAddress(Address(serverFactory->getServerAddress(), 5701));
                    }
                    return clientConfig;
                }

                std::auto_ptr<HazelcastClient> getNewClient() {
                    std::auto_ptr<HazelcastClient> result(new HazelcastClient(*getConfig()));
                    return result;
                }

            private:
                const HazelcastServerFactory *serverFactory;
            };


        }
    }
}

#endif //HAZELCASTCLIENT_CLIENTTESTSUPPORT_H
