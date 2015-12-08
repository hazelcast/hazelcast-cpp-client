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

#include "protocol/ClientMessageTest.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace protocol {
                using namespace iTest;

                ClientMessageTest::ClientMessageTest()
                        : iTestFixture<ClientMessageTest>("ClientMessageTest") {

                }

                void ClientMessageTest::addTests() {
                    addTest(&ClientMessageTest::testMessageFields, "Test Client Message Fields");
                }

                void ClientMessageTest::beforeClass() {

                }

                void ClientMessageTest::afterClass() {

                }

                void ClientMessageTest::beforeTest() {

                }

                void ClientMessageTest::afterTest() {

                }

                void ClientMessageTest::testMessageFields() {
                    std::auto_ptr<hazelcast::client::protocol::ClientMessage> msg =
                            hazelcast::client::protocol::ClientMessage::createForEncode(
                                    hazelcast::client::protocol::ClientMessage::HEADER_SIZE);

                    ASSERT_EQUAL(0, msg->getDataSize());

                    ASSERT_EQUAL(false, msg->isRetryable());

                    ASSERT_EQUAL(false, msg->isBindToSingleConnection());

                    ASSERT_EQUAL(hazelcast::client::protocol::ClientMessage::HEADER_SIZE, msg->getFrameLength());

                    msg->setIsBoundToSingleConnection(true);
                    msg->setRetryable(true);
                    msg->setCorrelationId(0xABCDEF12);
                    msg->setFlags(0x05);
                    msg->setMessageType(0xABCD);
                    msg->setPartitionId(0x8ABCDEF1);
                    msg->setVersion(4);
                    msg->updateFrameLength();

                    ASSERT_EQUAL(true, msg->isBindToSingleConnection());
                    ASSERT_EQUAL(true, msg->isRetryable());
                    ASSERT_EQUAL(0xABCDEF12, msg->getCorrelationId());
                    ASSERT_EQUAL(false, msg->isFlagSet(2));
                    ASSERT_EQUAL(true, msg->isFlagSet(4));
                    ASSERT_EQUAL(true, msg->isFlagSet(0x05));
                    ASSERT_EQUAL(0xABCD, msg->getMessageType());
                    ASSERT_EQUAL((int32_t)0x8ABCDEF1, msg->getPartitionId());
                    ASSERT_EQUAL(4, msg->getVersion());
                    int32_t size = hazelcast::client::protocol::ClientMessage::HEADER_SIZE;
                    ASSERT_EQUAL(size, msg->getFrameLength());
                    ASSERT_EQUAL(0, msg->getDataSize());

                    SocketStub sock;
                    ASSERT_EQUAL(size, msg->writeTo(sock, 0, size));
                }

                ClientMessageTest::SocketStub::SocketStub() : Socket(-1) {
                }

                ClientMessageTest::SocketStub::~SocketStub() {
                }


                int ClientMessageTest::SocketStub::send(const void *buf, int size) const {
                    ASSERT_EQUAL(hazelcast::client::protocol::ClientMessage::HEADER_SIZE, size);

                    byte expectedBytes[hazelcast::client::protocol::ClientMessage::HEADER_SIZE] = {
                            0x12, 0x0, 0x0, 0x0, // Frame length
                            0x04, // Version
                            0x05, // Flags
                            0xCD, 0xAB, // Message Type
                            0x12, 0xEF, 0xCD, 0xAB, // Correlation Id
                            0xF1, 0xDE, 0xBC, 0x8A, // Partition Id
                            0x12, 0x0 // Data Offset
                    };

                    ASSERT_EQUAL(0, memcmp(expectedBytes, buf, hazelcast::client::protocol::ClientMessage::HEADER_SIZE));
                    return size;
                }
            }
        }
    }
}
