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
// Created by sancar koyunlu on 8/26/13.


#ifndef HAZELCAST_HazelcastServerFactory
#define HAZELCAST_HazelcastServerFactory

#include <memory>

#include "hazelcast/client/Address.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace test {

            class HazelcastServer;

            class HazelcastServerFactory {
                friend class HazelcastServer;

                enum {
                    OK = 5678,
                    FAIL = -1,
                    END = 1,
                    START = 2,
                    SHUTDOWN = 3,
                    SHUTDOWN_ALL = 4,
                    START_SSL = 5
                };
            public:
                HazelcastServerFactory(const char* hostAddress);

                const std::string& getServerAddress() const;

                void shutdownAll();

                int getInstanceId(int retryNumber = 0, bool useSSL = false);

                ~HazelcastServerFactory();

            private:
                Address address;
                std::auto_ptr<Socket> socket;
                connection::OutputSocketStream outputSocketStream;
                connection::InputSocketStream inputSocketStream;
                util::ILogger &logger;

                void shutdownInstance(int id);
            };
        }
    }
}

#endif //HAZELCAST_HazelcastServerFactory

