/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/post.hpp>

#include "hazelcast/client/spi/lifecycle_service.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/util/hz_thread_pool.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace client {
class client_properties;

namespace spi {
namespace impl {
class HAZELCAST_API ClientExecutionServiceImpl
  : public std::enable_shared_from_this<ClientExecutionServiceImpl>
{
public:
    ClientExecutionServiceImpl(const std::string& name,
                               const client_properties& properties,
                               int32_t user_pool_size,
                               spi::lifecycle_service& service);

    void start();

    void shutdown();

    template<typename CompletionToken>
    void execute(CompletionToken token)
    {
        boost::asio::post(internal_executor_->get_executor(), token);
    }

    template<typename CompletionToken>
    std::shared_ptr<boost::asio::steady_timer> schedule(
      CompletionToken token,
      const std::chrono::milliseconds& delay)
    {
        return schedule_with_repetition(token, delay, std::chrono::seconds(0));
    }

    template<typename CompletionToken>
    std::shared_ptr<boost::asio::steady_timer> schedule_with_repetition(
      CompletionToken token,
      const std::chrono::milliseconds& delay,
      const std::chrono::milliseconds& period)
    {
        auto timer = std::make_shared<boost::asio::steady_timer>(
          internal_executor_->get_executor());
        return schedule_with_repetition_internal(token, delay, period, timer);
    }

    static void shutdown_thread_pool(hazelcast::util::hz_thread_pool* pool);

    util::hz_thread_pool& get_user_executor();
    util::hz_thread_pool& get_schema_replication_executor();

private:
    std::unique_ptr<util::hz_thread_pool> internal_executor_;
    std::unique_ptr<util::hz_thread_pool> user_executor_;
    std::unique_ptr<util::hz_thread_pool> schema_replication_executor_;
    spi::lifecycle_service& lifecycle_service_;
    const client_properties& client_properties_;
    int32_t user_pool_size_;

    template<typename CompletionToken>
    std::shared_ptr<boost::asio::steady_timer>
    schedule_with_repetition_internal(
      CompletionToken token,
      const std::chrono::milliseconds& delay,
      const std::chrono::milliseconds& period,
      std::shared_ptr<boost::asio::steady_timer> timer)
    {
        if (delay.count() > 0) {
            timer->expires_from_now(delay);
        } else {
            timer->expires_from_now(period);
        }

        timer->async_wait([=](boost::system::error_code ec) {
            if (ec) {
                return;
            }

            try {
                token();
            } catch (std::exception&) {
                assert(false);
            }

            if (lifecycle_service_.is_running() && period.count()) {
                schedule_with_repetition_internal(
                  token, std::chrono::seconds(-1), period, timer);
            }
        });
        return timer;
    }
};
} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
