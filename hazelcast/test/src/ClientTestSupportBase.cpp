/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include "ClientTestSupportBase.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            std::string ClientTestSupportBase::getCAFilePath() {
                return "hazelcast/test/resources/cpp_client.crt";
            }

            std::auto_ptr<hazelcast::client::ClientConfig> ClientTestSupportBase::getConfig() {
                std::auto_ptr<hazelcast::client::ClientConfig> clientConfig(new ClientConfig());
                return clientConfig;
            }

            std::auto_ptr<HazelcastClient> ClientTestSupportBase::getNewClient() {
                std::auto_ptr<HazelcastClient> result(new HazelcastClient(*getConfig()));
                return result;
            }

            const std::string ClientTestSupportBase::getSslFilePath() {
                return "hazelcast/test/resources/hazelcast-ssl.xml";
            }
        }
    }
}
