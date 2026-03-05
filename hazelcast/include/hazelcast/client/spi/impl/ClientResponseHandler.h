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
#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "hazelcast/util/export.h"

namespace hazelcast {
class logger;

namespace client {
namespace protocol {
class ClientMessage;
}
namespace spi {
namespace impl {
class ClientInvocationServiceImpl;

struct ResponseEntry
{
    int64_t correlation_id;
    std::shared_ptr<protocol::ClientMessage> message;
};

class HAZELCAST_API ClientResponseHandler
{
public:
    ClientResponseHandler(ClientInvocationServiceImpl& invocation_service,
                          logger& lg,
                          int thread_count);

    void start();

    void shutdown();

    /// Called by IO thread to enqueue a response for async processing.
    /// Distributes across response threads by correlation_id % thread_count.
    void enqueue(int64_t correlation_id,
                 std::shared_ptr<protocol::ClientMessage> message);

private:
    struct ResponseQueue
    {
        std::mutex mutex;
        std::condition_variable cv;
        std::deque<ResponseEntry> entries;
    };

    void run(int thread_index);

    void process_response(const ResponseEntry& entry);

    ClientInvocationServiceImpl& invocation_service_;
    logger& logger_;
    int thread_count_;
    std::vector<std::unique_ptr<ResponseQueue>> queues_;
    std::vector<std::thread> threads_;
    std::atomic<bool> running_{ false };
};

} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast
