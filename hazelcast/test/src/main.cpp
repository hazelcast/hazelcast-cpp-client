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

#include "HazelcastServerFactory.h"
#include "SimpleMapTest.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>

using namespace hazelcast::client::test;

void testSpeed(const char* address) {
    SimpleMapTest s(address, 5701);
    s.run();
}

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServerFactory *g_srvFactory = NULL;
        }
    }
}

class ServerFactoryEnvironment : public ::testing::Environment {
public:
    ServerFactoryEnvironment(const char *srvAddress) : serverAddress(srvAddress) {
    }

    void SetUp() {
        hazelcast::client::test::g_srvFactory = new HazelcastServerFactory(serverAddress);
    }

    void TearDown() {
        delete hazelcast::client::test::g_srvFactory;
    }

private :
    const char *serverAddress;
};

int main(int argc, char** argv) {
    const char* address;

    testing::InitGoogleTest(&argc, argv);

    if(argc == 2){
        address = argv[1];
    } else {
        address = "127.0.0.1";
    }
    std::cout << "Server address : "  << address << std::endl;

    ::testing::AddGlobalTestEnvironment(new ServerFactoryEnvironment(address));

    return RUN_ALL_TESTS();

    //SimpleMapTest(address, 5701).run();
}

