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
#include <atomic>
#include <boost/asio.hpp>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/util/SynchronizedMap.h"

#include "hazelcast/util/Closeable.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/AtomicBoolean.h"
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

        class SocketInterceptor;

        namespace connection {
            class AuthenticationFuture;

            class ClientConnectionManagerImpl;

            class HAZELCAST_API Connection : public util::Closeable, public std::enable_shared_from_this<Connection> {
            public:
                Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                           const std::shared_ptr<AuthenticationFuture> &authFuture,
                           internal::socket::SocketFactory &socketFactory,
                           boost::asio::io_context &ioContext, bool asOwner,
                           ClientConnectionManagerImpl &clientConnectionManager, int64_t connectTimeoutInMillis);

                virtual ~Connection();

                void asyncStart();

                void close();

                void close(const std::string &reason);

                void close(const std::string &reason, const std::shared_ptr<exception::IException> &cause);

                bool write(const std::shared_ptr<protocol::ClientMessage> &message);

                const std::shared_ptr<Address> &getRemoteEndpoint() const;

                void setRemoteEndpoint(const std::shared_ptr<Address> &remoteEndpoint);

                bool isAuthenticatedAsOwner();

                void setIsAuthenticatedAsOwner();

                virtual void handleClientMessage(const std::shared_ptr<protocol::ClientMessage> &message);

                int getConnectionId() const;

                bool isAlive();

                int64_t lastReadTimeMillis();

                const std::string &getCloseReason() const;

                bool operator==(const Connection &rhs) const;

                bool operator!=(const Connection &rhs) const;

                bool operator<(const Connection &rhs) const;

                const std::string &getConnectedServerVersionString() const;

                void setConnectedServerVersion(const std::string &connectedServerVersionString);

                std::unique_ptr<Address> getLocalSocketAddress() const;

                int getConnectedServerVersion() const;

                int64_t getStartTimeInMillis() const;

                const Socket &getSocket() const;

                void authenticate();

                void reAuthenticateAsOwner();

                friend std::ostream &operator<<(std::ostream &os, const Connection &connection);

                ReadHandler readHandler;
            private:
                void logClose();

                void innerClose();

                int64_t startTimeInMillis;
                std::atomic<int64_t> closedTimeMillis;
                spi::ClientContext &clientContext;
                protocol::IMessageHandler &invocationService;
                std::unique_ptr<Socket> socket;
                std::shared_ptr<AuthenticationFuture> authFuture;
                util::AtomicBoolean authenticatedAsOwner;

                int connectionId;
                std::string closeReason;
                std::shared_ptr<exception::IException> closeCause;

                std::string connectedServerVersionString;
                int connectedServerVersion;

                std::shared_ptr<Address> remoteEndpoint;

                util::ILogger &logger;
                bool asOwner;
                ClientConnectionManagerImpl &connectionManager;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION

