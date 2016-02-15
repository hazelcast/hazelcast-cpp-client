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
// Created by İhsan Demir on 19/05/15.
//

#ifndef HAZELCAST_CLIENT_TEST_PROTOCOL_CLIENTMESSAGETEST_H
#define HAZELCAST_CLIENT_TEST_PROTOCOL_CLIENTMESSAGETEST_H

#include <gtest/gtest.h>
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace protocol {
                static int numBytes;
                static byte buffer[100];

                class ClientMessageTest : public ::testing::Test {
                protected:
                    class SocketStub : public Socket {
                    public:
                        virtual int send(const void *buf, int size) const;

                        virtual ~SocketStub();

                        SocketStub();
                    };
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_PROTOCOL_CLIENTMESSAGETEST_H
