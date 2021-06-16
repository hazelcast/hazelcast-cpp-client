/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace hazelcast::client::test;

namespace hazelcast {
namespace client {
namespace test {
HazelcastServerFactory* g_srvFactory = nullptr;
std::shared_ptr<RemoteControllerClient> remoteController;
} // namespace test
} // namespace client
} // namespace hazelcast

class ServerFactoryEnvironment : public ::testing::Environment
{
public:
    ServerFactoryEnvironment() = default;

    void SetUp() override {}

    void TearDown() override { delete hazelcast::client::test::g_srvFactory; }
};

int
main(int argc, char** argv)
{
    const char* serverAddress = "127.0.0.1";
    int port = 9701;

    auto transport =
      std::make_shared<TBufferedTransport>(std::make_shared<TSocket>(serverAddress, port));
    try {
        transport->open();
    } catch (apache::thrift::transport::TTransportException& e) {
        std::cerr << "Failed to open connection to remote controller server at address "
                  << serverAddress << ":" << port << ". The exception: " << e.what() << std::endl;
        exit(-1);
    }

    remoteController =
      std::make_shared<RemoteControllerClient>(std::make_shared<TBinaryProtocol>(transport));

    g_srvFactory =
      new HazelcastServerFactory(serverAddress, "hazelcast/test/resources/hazelcast.xml");

    ::testing::AddGlobalTestEnvironment(new ServerFactoryEnvironment);

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
