/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <atomic>
#include <chrono>

#include "hazelcast/util/export.h"
#include "hazelcast/client/protocol/IMessageHandler.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace client {
namespace spi {
class ClientContext;
namespace impl {
class HAZELCAST_API ClientInvocationServiceImpl
  : public protocol::IMessageHandler
{
public:
    explicit ClientInvocationServiceImpl(ClientContext& client);

    void start();

    void shutdown();

    bool invoke_on_partition_owner(
      const std::shared_ptr<ClientInvocation>& invocation,
      int partition_id);

    bool invoke(std::shared_ptr<ClientInvocation> invocation);

    bool invoke_on_target(const std::shared_ptr<ClientInvocation>& invocation,
                          boost::uuids::uuid uuid);

    bool invoke_on_connection(
      const std::shared_ptr<ClientInvocation>& invocation,
      const std::shared_ptr<connection::Connection>& connection);

    void check_invocation_allowed();

    void check_urgent_invocation_allowed(const ClientInvocation&);

    bool is_smart_routing() const;

    std::chrono::milliseconds get_invocation_timeout() const;

    std::chrono::milliseconds get_invocation_retry_pause() const;

    bool is_redo_operation();

    void handle_client_message(
      const std::shared_ptr<ClientInvocation>& invocation,
      const std::shared_ptr<protocol::ClientMessage>& response) override;

    const std::chrono::milliseconds& get_backup_timeout() const;

    bool fail_on_indeterminate_state() const;

    void add_backup_listener();

private:
    class BackupListenerMessageCodec : public ListenerMessageCodec
    {
    public:
        protocol::ClientMessage encode_add_request(
          bool local_only) const override;

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override;
    };

    class noop_backup_event_handler
      : public protocol::codec::client_localbackuplistener_handler
    {
    public:
        explicit noop_backup_event_handler(logger& l);
        void handle_backup(int64_t source_invocation_correlation_id) override;
    };

    ClientContext& client_;
    logger& logger_;
    std::atomic<bool> is_shutdown_{ false };
    std::chrono::milliseconds invocation_timeout_;
    std::chrono::milliseconds invocation_retry_pause_;
    bool smart_routing_;
    bool backup_acks_enabled_;
    bool fail_on_indeterminate_operation_state_;
    std::chrono::milliseconds backup_timeout_;

    static void write_to_connection(
      connection::Connection& connection,
      const std::shared_ptr<ClientInvocation>& client_invocation);

    bool send(const std::shared_ptr<impl::ClientInvocation>& invocation,
              const std::shared_ptr<connection::Connection>& connection);
};
} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
