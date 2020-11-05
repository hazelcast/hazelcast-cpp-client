/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <memory>
#include <ostream>
#include <stdint.h>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/logger.h"

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
        }

        namespace internal {
            namespace socket {
                class SocketFactory;
            }
        }

        class Address;

        class SocketInterceptor;

        namespace connection {
            class ConnectionFuture;

            class ClientConnectionManagerImpl;

            class HAZELCAST_API Connection : public util::Closeable, public std::enable_shared_from_this<Connection> {
            public:
                Connection(const Address &address, spi::ClientContext &clientContext, int32_t connectionId,
                           internal::socket::SocketFactory &socketFactory,
                           ClientConnectionManagerImpl &clientConnectionManager,
                           std::chrono::milliseconds &connectTimeoutInMillis);

                ~Connection() override;

                void connect();

                void close();

                void close(const std::string &reason) override;

                void close(const std::string &reason, std::exception_ptr cause);

                void write(const std::shared_ptr<spi::impl::ClientInvocation> &clientInvocation);

                const boost::optional<Address> &get_remote_address() const;

                void set_remote_address(boost::optional<Address> endpoint);

                boost::uuids::uuid get_remote_uuid() const;

                void set_remote_uuid(boost::uuids::uuid remoteUuid);

                void handle_client_message(const std::shared_ptr<protocol::ClientMessage> &message);

                int get_connection_id() const;

                bool is_alive() const;

                std::chrono::steady_clock::time_point last_read_time() const;

                const std::string &get_close_reason() const;

                bool operator==(const Connection &rhs) const;

                bool operator!=(const Connection &rhs) const;

                bool operator<(const Connection &rhs) const;

                const std::string &get_connected_server_version_string() const;

                void set_connected_server_version(const std::string &connectedServer);

                boost::optional<Address> get_local_socket_address() const;

                std::chrono::system_clock::time_point get_start_time() const;

                Socket &get_socket();

                void deregister_invocation(int64_t callId);

                friend std::ostream &operator<<(std::ostream &os, const Connection &connection);

                ReadHandler read_handler;
                std::unordered_map<int64_t, std::shared_ptr<spi::impl::ClientInvocation>> invocations;
            private:
                void log_close();

                void inner_close();

                std::chrono::system_clock::time_point startTime_;
                std::atomic<std::chrono::milliseconds> closedTimeDuration_;
                spi::ClientContext &clientContext_;
                protocol::IMessageHandler &invocationService_;
                std::unique_ptr<Socket> socket_;
                int32_t connectionId_;
                std::string closeReason_;
                std::exception_ptr closeCause_;
                std::string connectedServerVersionString_;
                // TODO: check if they need to be atomic
                boost::optional<Address> remote_address_;
                boost::uuids::uuid remote_uuid_;
                logger &logger_;
                std::atomic_bool alive_;
                std::unique_ptr<boost::asio::steady_timer> backup_timer_;

                void schedule_periodic_backup_cleanup(std::chrono::milliseconds backupTimeout,
                                                      std::shared_ptr<Connection> this_connection);
            };
        }
    }
}

namespace std {
    template<>
    struct hash<std::shared_ptr<hazelcast::client::connection::Connection>> {
        std::size_t operator()(const std::shared_ptr<hazelcast::client::connection::Connection> &conn) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



