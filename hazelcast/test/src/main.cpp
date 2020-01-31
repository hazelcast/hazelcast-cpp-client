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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace hazelcast::client::test;

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServerFactory *g_srvFactory = NULL;
            std::shared_ptr<RemoteControllerClient> remoteController;
        }
    }
}

class ServerFactoryEnvironment : public ::testing::Environment {
public:
    ServerFactoryEnvironment(const char *srvAddress) : serverAddress(srvAddress) {
    }

    void SetUp() {
        int port = 9701;
        auto transport = make_shared<TBufferedTransport>(make_shared<TSocket>(serverAddress, port));
        try {
            transport->open();
        } catch (apache::thrift::transport::TTransportException &e) {
            cerr << "Failed to open connection to remote controller server at address " << serverAddress << ":"
                 << port << ". The exception: " << e.what() << endl;
            exit(-1);
        }

        remoteController = make_shared<RemoteControllerClient>(make_shared<TBinaryProtocol>(transport));

        hazelcast::client::test::g_srvFactory = new HazelcastServerFactory(serverAddress, "hazelcast/test/resources/hazelcast.xml");
    }

    void TearDown() {
        delete hazelcast::client::test::g_srvFactory;
    }

private:
    const char *serverAddress;
};

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new ServerFactoryEnvironment("127.0.0.1"));

    return RUN_ALL_TESTS();
}

