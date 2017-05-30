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
// Created by İhsan Demir on 26/05/15.
//

#ifndef HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H
#define HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H

#include <memory>

namespace hazelcast {
    namespace client {
        class ClientConfig;

        class HazelcastClient;

        namespace test {
            class HazelcastServerFactory;

            extern HazelcastServerFactory *g_srvFactory;

            class ClientTestSupportBase {
            public:
                static std::string getCAFilePath();
            protected:

                static std::auto_ptr<hazelcast::client::ClientConfig> getConfig();

                static std::auto_ptr<HazelcastClient> getNewClient();
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H
