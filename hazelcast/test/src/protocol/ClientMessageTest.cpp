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
//
// Created by İhsan Demir on 19/05/15.
//

#include "protocol/ClientMessageTest.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace protocol {
                TEST_F(ClientMessageTest, testMessageFields) {
                    std::auto_ptr<hazelcast::client::protocol::ClientMessage> msg =
                            hazelcast::client::protocol::ClientMessage::createForEncode(22);
                    ASSERT_EQ(0, msg->getDataSize());

                    ASSERT_FALSE(msg->isRetryable());

                    ASSERT_FALSE(msg->isBindToSingleConnection());

                    ASSERT_EQ(22, msg->getFrameLength());

                    msg->setIsBoundToSingleConnection(true);
                    msg->setRetryable(true);
                    msg->setCorrelationId(0xABCDEF12);
                    msg->addFlag(0x05);
                    msg->setMessageType(0xABCD);
                    msg->setPartitionId(0x8ABCDEF1);
                    msg->setVersion(4);
                    msg->updateFrameLength();

                    ASSERT_TRUE(msg->isBindToSingleConnection());
                    ASSERT_TRUE(msg->isRetryable());
                    ASSERT_EQ(0xABCDEF12, msg->getCorrelationId());
                    ASSERT_FALSE(msg->isFlagSet(2));
                    ASSERT_TRUE(msg->isFlagSet(4));
                    ASSERT_TRUE(msg->isFlagSet(0x05));
                    ASSERT_EQ(0xABCD, msg->getMessageType());
                    ASSERT_EQ((int32_t)0x8ABCDEF1, msg->getPartitionId());
                    ASSERT_EQ(4, msg->getVersion());
                    ASSERT_EQ(22, msg->getFrameLength());
                    ASSERT_EQ(0, msg->getDataSize());

                    SocketStub sock;
                    ASSERT_EQ(22, msg->writeTo(sock, 0, 22));

                    ASSERT_EQ(22, numBytes);

                    byte expectedBytes[22] = {
                            0x16, 0x0, 0x0, 0x0, // Frame length
                            0x04, // Version
                            0x05, // Flags
                            0xCD, 0xAB, // Message Type
                            0x12, 0xEF, 0xCD, 0xAB, 0x00, 0x00, 0x00, 0x00, // Correlation Id
                            0xF1, 0xDE, 0xBC, 0x8A, // Partition Id
                            0x16, 0x0 // Data Offset
                    };

                    ASSERT_EQ(0, memcmp(expectedBytes, buffer, 22));

                }

                ClientMessageTest::SocketStub::SocketStub() : internal::socket::TcpSocket(-1) {
                }

                ClientMessageTest::SocketStub::~SocketStub() {
                }

                int ClientMessageTest::SocketStub::send(const void *buf, int size, int flag) {
                    if (size <= 100) {
                        numBytes = size;
                        memcpy(buffer, buf, (size_t)numBytes);
                    } else {
                        numBytes = 0;
                    }
                    return numBytes;
                }
            }
        }
    }
}
