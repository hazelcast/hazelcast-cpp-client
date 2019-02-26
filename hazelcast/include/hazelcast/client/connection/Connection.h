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
//
// Created by sancar koyunlu on 5/21/13.
#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include <memory>
#include <ostream>
#include <stdint.h>
#include <boost/enable_shared_from_this.hpp>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/util/ILogger.h"
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
        namespace exception {
            class IException;
        }
        namespace spi {
            class ClientContext;

            class ClientInvocationService;
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

            class HAZELCAST_API Connection : public util::Closeable, public boost::enable_shared_from_this<Connection> {
            public:
                Connection(const Address& address, spi::ClientContext& clientContext, int connectionId, InSelector& iListener,
                           OutSelector& listener, internal::socket::SocketFactory &socketFactory);

                virtual ~Connection();

                void connect(int timeoutInMillis);

                void close(const char *reason = NULL);

                void close(const char *reason, const boost::shared_ptr<exception::IException> &cause);

                bool write(const boost::shared_ptr<protocol::ClientMessage> &message);

                const boost::shared_ptr<Address> &getRemoteEndpoint() const;

                void setRemoteEndpoint(const boost::shared_ptr<Address> &remoteEndpoint);

                Socket& getSocket();

                ReadHandler& getReadHandler();

                WriteHandler& getWriteHandler();

                bool isAuthenticatedAsOwner();

                void setIsAuthenticatedAsOwner();

                virtual void handleClientMessage(const boost::shared_ptr<protocol::ClientMessage> &message);

                int getConnectionId() const;

                bool isAlive();

                int64_t lastReadTimeMillis();

                const std::string &getCloseReason() const;

                bool operator==(const Connection &rhs) const;

                bool operator!=(const Connection &rhs) const;

                bool operator<(const Connection &rhs) const;

                const std::string &getConnectedServerVersionString() const;

                void setConnectedServerVersion(const std::string &connectedServerVersionString);

                std::auto_ptr<Address> getLocalSocketAddress() const;

                int getConnectedServerVersion() const;

                int64_t getStartTimeInMillis() const;

                friend std::ostream &operator<<(std::ostream &os, const Connection &connection);

            private:
                void logClose();

                void innerClose();

                int64_t startTimeInMillis;
                util::Atomic<int64_t> closedTimeMillis;
                spi::ClientContext& clientContext;
                protocol::IMessageHandler &invocationService;
                std::auto_ptr<Socket> socket;
                ReadHandler readHandler;
                WriteHandler writeHandler;
                util::AtomicBoolean authenticatedAsOwner;

                int connectionId;
                std::string closeReason;
                boost::shared_ptr<exception::IException> closeCause;

                std::string connectedServerVersionString;
                int connectedServerVersion;

                boost::shared_ptr<Address> remoteEndpoint;

                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION

