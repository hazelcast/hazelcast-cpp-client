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
ClientResponseHandler::enqueue(int64_t correlation_id,
                               std::shared_ptr<protocol::ClientMessage> message)
{
    auto index = static_cast<size_t>(correlation_id >= 0 ? correlation_id
                                                         : -correlation_id) %
                 queues_.size();
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

        if (protocol::codec::ErrorCodec::EXCEPTION_MESSAGE_TYPE ==
            entry.message->get_message_type()) {
            auto error_holder =
              protocol::codec::ErrorCodec::decode(*entry.message);
            invocation->notify_exception(
              invocation_service_.get_client_context()
                .get_client_exception_factory()
                .create_exception(error_holder));
        } else {
            invocation->notify(entry.message);
        }

        invocation_service_.deregister_invocation(entry.correlation_id);
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
