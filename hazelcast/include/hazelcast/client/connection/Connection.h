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
// Created by sancar koyunlu on 5/21/13.
#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include <memory>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;

            class InvocationService;
        }

        namespace internal {
            namespace socket {
                class SocketFactory;
            }
        }

        class Address;

        namespace connection {
            class OutSelector;

            class InSelector;

            class HAZELCAST_API Connection : public util::Closeable, public protocol::IMessageHandler {
            public:
                Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener,
                           OutSelector& listener, internal::socket::SocketFactory &socketFactory, bool isOwner);

                ~Connection();

                void init(const std::vector<byte>& PROTOCOL);

                void connect(int timeoutInMillis);

                void close();

                void write(protocol::ClientMessage *message);

                const Address& getRemoteEndpoint() const;

                void setRemoteEndpoint(const Address& remoteEndpoint);

                Socket& getSocket();

                std::auto_ptr<protocol::ClientMessage> sendAndReceive(protocol::ClientMessage &clientMessage);

                ReadHandler& getReadHandler();

                WriteHandler& getWriteHandler();

                void setAsOwnerConnection(bool isOwnerConnection);

                void writeBlocking(protocol::ClientMessage &packet);

                std::auto_ptr<protocol::ClientMessage> readBlocking();

                bool isHeartBeating();

                void heartBeatingFailed();

                void heartBeatingSucceed();

                bool isOwnerConnection() const;

                virtual void handleMessage(connection::Connection &connection, std::auto_ptr<protocol::ClientMessage> message);

                int getConnectionId() const;

                void setConnectionId(int connectionId);

                util::Atomic<time_t> lastRead;
                util::AtomicBoolean live;
            private:
                spi::ClientContext& clientContext;
                spi::InvocationService& invocationService;
                std::auto_ptr<Socket> socket;
                ReadHandler readHandler;
                WriteHandler writeHandler;
                bool _isOwnerConnection;
                util::AtomicBoolean heartBeating;
                byte* receiveBuffer;
                util::ByteBuffer receiveByteBuffer;

                protocol::ClientMessageBuilder messageBuilder;
                protocol::ClientMessage wrapperMessage;
                std::auto_ptr<protocol::ClientMessage> responseMessage;

                int connectionId;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION

