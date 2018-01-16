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


#ifndef HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_
#define HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

#include <boost/shared_ptr.hpp>

#include "RemoteController.h"
#include "remotecontroller_types.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace test {

            class HazelcastServerFactory {
            public:
                HazelcastServerFactory(const std::string &serverXmlConfigFilePath);

                const std::string& getServerAddress() const;

                void startServer(Member &member);

                void setAttributes(Member &member);

                void shutdownServer(Member &member);

                ~HazelcastServerFactory();

                static void init(const std::string &serverAddress);

            private:
                void shutdownAll();

                std::string serverAddress;
                util::ILogger &logger;
                static boost::shared_ptr<RemoteControllerClient> rcClient;
                ::Cluster cluster;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

