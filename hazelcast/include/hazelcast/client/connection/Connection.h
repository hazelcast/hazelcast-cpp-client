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
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/Closeable.h"

namespace hazelcast {
    namespace client {

        namespace impl {
            class ClientRequest;
        }
        namespace spi {
            class ClientContext;

            class InvocationService;
        }
        namespace serialization {
            namespace pimpl {
                class SerializationService;

                class Data;

                class Packet;

                class PortableContext;
            }

        }

        class Address;

        namespace connection {
            class CallPromise;

            class ClientResponse;

            class OutSelector;

            class InSelector;

            class Connection : public util::Closeable {
            public:
                Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener, OutSelector& listener, bool isOwner);

                ~Connection();

                void init(const std::vector<byte>& PROTOCOL);

                void connect(int timeoutInMillis);

                void close();

                void write(serialization::pimpl::Packet *packet);

                const Address& getRemoteEndpoint() const;

                void setRemoteEndpoint(Address& remoteEndpoint);

                Socket& getSocket();

                boost::shared_ptr<connection::ClientResponse> sendAndReceive(const impl::ClientRequest& clientRequest);

                ReadHandler& getReadHandler();

                WriteHandler& getWriteHandler();

                void setAsOwnerConnection(bool isOwnerConnection);

                void writeBlocking(serialization::pimpl::Packet const& packet);

                serialization::pimpl::Packet readBlocking();

                bool isHeartBeating();

                void heartBeatingFailed();

                void heartBeatingSucceed();

                bool isOwnerConnection() const;

                util::AtomicInt lastRead;
                util::AtomicBoolean live;
            private:
                spi::ClientContext& clientContext;
                spi::InvocationService& invocationService;
                Socket socket;
                ReadHandler readHandler;
                WriteHandler writeHandler;
                bool _isOwnerConnection;
                util::AtomicBoolean heartBeating;
                char* receiveBuffer;
                util::ByteBuffer receiveByteBuffer;
            };

        }
    }
}

#endif //HAZELCAST_CONNECTION

