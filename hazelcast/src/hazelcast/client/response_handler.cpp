/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/format.hpp>

#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/spi/impl/ClientInvocationServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientResponseHandler.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/logger.h"

namespace hazelcast {
namespace client {
namespace spi {
namespace impl {

ClientResponseHandler::ClientResponseHandler(
  ClientInvocationServiceImpl& invocation_service,
  logger& lg,
  int thread_count)
  : invocation_service_(invocation_service)
  , logger_(lg)
  , thread_count_(thread_count)
{
    util::Preconditions::check_positive(
      thread_count_,
      client_properties::RESPONSE_THREAD_COUNT + " must be positive");
}

void
ClientResponseHandler::start()
{
    running_.store(true);
    queues_.reserve(thread_count_);
    for (int i = 0; i < thread_count_; ++i) {
        queues_.push_back(std::make_unique<ResponseQueue>());
    }
    threads_.reserve(thread_count_);
    for (int i = 0; i < thread_count_; ++i) {
        threads_.emplace_back([this, i]() { run(i); });
    }
}

void
ClientResponseHandler::shutdown()
{
    running_.store(false);
    for (auto& q : queues_) {
        std::lock_guard<std::mutex> lock(q->mutex);
        q->cv.notify_all();
    }
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads_.clear();
    queues_.clear();
}

void
ClientResponseHandler::accept(std::shared_ptr<protocol::ClientMessage> message)
{
    auto correlation_id = message->get_correlation_id();
    // AbstractCallIdSequence::next() monotonically increases.
    // if it goes beyond int64_t MAX, it will wrap to negative number.
    auto index = std::abs(correlation_id) % queues_.size();
    auto& q = *queues_[index];
    {
        std::lock_guard<std::mutex> lock(q.mutex);
        q.entries.push_back(
          ResponseEntry{ correlation_id, std::move(message) });
    }
    q.cv.notify_one();
}

void
ClientResponseHandler::run(int thread_index)
{
    auto& q = *queues_[thread_index];
    while (running_.load()) {
        ResponseEntry entry;
        {
            std::unique_lock<std::mutex> lock(q.mutex);
            q.cv.wait(lock,
                      [&]() { return !q.entries.empty() || !running_.load(); });
            if (!running_.load() && q.entries.empty()) {
                return;
            }
            entry = std::move(q.entries.front());
            q.entries.pop_front();
        }
        process_response(entry);
    }
    // Drain remaining entries on shutdown
    std::lock_guard<std::mutex> lock(q.mutex);
    for (auto& e : q.entries) {
        process_response(e);
    }
}

void
ClientResponseHandler::process_response(const ResponseEntry& entry)
{
    try {
        auto flags = entry.message->get_header_flags();

        // Handle backup events first — the header correlation ID belongs to
        // the backup listener registration which may no longer be in the
        // global invocation map.  The source invocation's correlation ID is
        // in the payload.
        if (entry.message->is_flag_set(
              flags, protocol::ClientMessage::BACKUP_EVENT_FLAG)) {
            entry.message->rd_ptr(protocol::ClientMessage::EVENT_HEADER_LEN);
            auto source_correlation_id = entry.message->get<int64_t>();
            auto source_invocation =
              invocation_service_.get_invocation(source_correlation_id);
            if (source_invocation) {
                source_invocation->notify_backup();
            }
            return;
        }

        auto invocation =
          invocation_service_.get_invocation(entry.correlation_id);
        if (!invocation) {
            HZ_LOG(
              logger_,
              warning,
              boost::str(
                boost::format(
                  "No invocation for correlationId: %1%. Dropping response.") %
                entry.correlation_id));
            return;
        }

        if (entry.message->is_flag_set(
              flags, protocol::ClientMessage::IS_EVENT_FLAG)) {
            invocation_service_.get_client_context()
              .get_client_listener_service()
              .handle_client_message(invocation, entry.message);
            return;
        }

        if (protocol::codec::ErrorCodec::EXCEPTION_MESSAGE_TYPE ==
            entry.message->get_message_type()) {
            auto error_holder =
              protocol::codec::ErrorCodec::decode(*entry.message);
            invocation->notify_exception(
              entry.correlation_id,
              invocation_service_.get_client_context()
                .get_client_exception_factory()
                .create_exception(error_holder));
        } else {
            invocation->notify(entry.message);
        }
    } catch (std::exception& e) {
        HZ_LOG(logger_,
               severe,
               boost::str(
                 boost::format(
                   "Failed to process response for correlationId %1%. %2%") %
                 entry.correlation_id % e.what()));
    }
}

} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast
