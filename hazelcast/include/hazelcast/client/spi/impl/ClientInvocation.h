/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <ostream>
#include <memory>
#include <atomic>

#include <boost/thread/future.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/asio/steady_timer.hpp>

#include "hazelcast/util/Sync.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace client {
class address;

namespace connection {
class Connection;
}

namespace spi {
class lifecycle_service;

class ClientContext;

namespace impl {
class ClientClusterServiceImpl;
class ClientInvocationServiceImpl;
class ClientExecutionServiceImpl;

namespace sequence {
class CallIdSequence;
}

/**
 * Handles the routing of a request from a Hazelcast client.
 * <p>
 * 1) Where should request be sent?<br>
 * 2) Should it be retried?<br>
 * 3) How many times is it retried?
 */
class HAZELCAST_API ClientInvocation
  : public std::enable_shared_from_this<ClientInvocation>
{
public:
    virtual ~ClientInvocation();

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      std::shared_ptr<protocol::ClientMessage>&& client_message,
      const std::string& object_name,
      int partition_id);

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      std::shared_ptr<protocol::ClientMessage>&& client_message,
      const std::string& object_name,
      const std::shared_ptr<connection::Connection>& connection = nullptr);

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      std::shared_ptr<protocol::ClientMessage>&& client_message,
      const std::string& object_name,
      boost::uuids::uuid uuid);

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      protocol::ClientMessage& client_message,
      const std::string& object_name,
      int partition_id);

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      protocol::ClientMessage& client_message,
      const std::string& object_name,
      const std::shared_ptr<connection::Connection>& connection = nullptr);

    static std::shared_ptr<ClientInvocation> create(
      spi::ClientContext& client_context,
      protocol::ClientMessage& client_message,
      const std::string& object_name,
      boost::uuids::uuid uuid);

    boost::future<protocol::ClientMessage> invoke();

    boost::future<protocol::ClientMessage> invoke_urgent();

    void run();

    virtual const std::string get_name() const;

    void notify(const std::shared_ptr<protocol::ClientMessage>& client_message);

    void notify_exception(std::exception_ptr exception);

    void notify_backup();

    std::shared_ptr<connection::Connection> get_send_connection() const;

    void wait_invoked() const;

    void set_send_connection(
      const std::shared_ptr<connection::Connection>& send_connection);

    std::shared_ptr<protocol::ClientMessage> get_client_message() const;

    const std::shared_ptr<EventHandler<protocol::ClientMessage>>&
    get_event_handler() const;

    void set_event_handler(
      const std::shared_ptr<EventHandler<protocol::ClientMessage>>&
        event_handler);

    friend std::ostream& operator<<(std::ostream& os,
                                    const ClientInvocation& invocation);

    boost::promise<protocol::ClientMessage>& get_promise();

    void detect_and_handle_backup_timeout(
      const std::chrono::milliseconds& backup_timeout);

private:
    static constexpr int MAX_FAST_INVOCATION_COUNT = 5;
    static constexpr int UNASSIGNED_PARTITION = -1;

    logger& logger_;
    lifecycle_service& lifecycle_service_;
    ClientInvocationServiceImpl& invocation_service_;
    std::shared_ptr<ClientExecutionServiceImpl> execution_service_;
    boost::atomic_shared_ptr<std::shared_ptr<protocol::ClientMessage>>
      client_message_;
    std::shared_ptr<sequence::CallIdSequence> call_id_sequence_;
    boost::uuids::uuid uuid_;
    int partition_id_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds retry_pause_;
    std::string object_name_;
    std::weak_ptr<connection::Connection> connection_;
    bool bound_to_single_connection_{ false };
    std::atomic_bool invoked_or_exception_set_{ false };
    boost::atomic_shared_ptr<std::weak_ptr<connection::Connection>>
      send_connection_;
    std::shared_ptr<EventHandler<protocol::ClientMessage>> event_handler_;
    std::atomic<int64_t> invoke_count_;
    boost::promise<protocol::ClientMessage> invocation_promise_;
    bool urgent_;
    bool smart_routing_;

    int32_t backup_acks_received_ = 0;

    /**
     * Number of expected backups. It is set correctly as soon as the pending
     * response is set.
     */
    int8_t backup_acks_expected_ = -1;

    /**
     * Contains the pending response from the primary. It is pending because it
     * could be that backups need to complete. Note that we do not need thread
     * safety since these are only read/write from the same io thread for the
     * connection.
     */
    std::shared_ptr<protocol::ClientMessage> pending_response_;

    /**
     * The time when the response of the primary has been received.
     */
    std::chrono::steady_clock::time_point pending_response_received_time_;

    std::shared_ptr<boost::asio::steady_timer> retry_timer_;

    ClientInvocation(
      spi::ClientContext& client_context,
      std::shared_ptr<protocol::ClientMessage>&& message,
      const std::string& name,
      int partition = UNASSIGNED_PARTITION,
      const std::shared_ptr<connection::Connection>& conn = nullptr,
      boost::uuids::uuid uuid = { 0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0,
                                  0x0 });

    void invoke_on_selection();

    bool is_bind_to_single_connection() const;

    void retry();

    bool should_retry(exception::iexception& exception);

    void execute();

    ClientInvocation(const ClientInvocation& rhs) = delete;

    void operator=(const ClientInvocation& rhs) = delete;

    std::shared_ptr<protocol::ClientMessage> copy_message();

    void set_exception(const std::exception& e,
                       boost::exception_ptr exception_ptr);

    void log_exception(exception::iexception& e);

    void erase_invocation() const;

    void complete(const std::shared_ptr<protocol::ClientMessage>& msg);

    void complete_with_pending_response();
};
} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
