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

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/post.hpp>

#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/hz_thread_pool.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class hz_thread_pool;
    }

    namespace client {
        class ClientProperties;

        namespace spi {
            namespace impl {
                class HAZELCAST_API ClientExecutionServiceImpl :
                        public std::enable_shared_from_this<ClientExecutionServiceImpl> {
                public:
                    ClientExecutionServiceImpl(const std::string &name, const ClientProperties &properties,
                                               int32_t poolSize, spi::LifecycleService &service);

                    void start();

                    void shutdown();

                    template<typename CompletionToken>
                    void execute(CompletionToken token) {
                        boost::asio::post(internalExecutor->get_executor(), token);
                    }

                    template<typename CompletionToken>
                    std::shared_ptr<boost::asio::steady_timer> schedule(CompletionToken token,
                                                                        const std::chrono::steady_clock::duration &delay) {
                        return scheduleWithRepetition(token, delay, std::chrono::seconds(0));
                    }

                    template<typename CompletionToken>
                    std::shared_ptr<boost::asio::steady_timer> scheduleWithRepetition(CompletionToken token,
                                                                                      const std::chrono::steady_clock::duration &delay,
                                                                                      const std::chrono::steady_clock::duration &period) {
                        auto timer = std::make_shared<boost::asio::steady_timer>(internalExecutor->get_executor());
                        return scheduleWithRepetitionInternal(token, delay, period, timer);
                    }

                    boost::asio::thread_pool::executor_type getUserExecutor() const;

                    static void shutdownThreadPool(hazelcast::util::hz_thread_pool *pool);
                private:
                    std::unique_ptr<hazelcast::util::hz_thread_pool> internalExecutor;
                    std::unique_ptr<hazelcast::util::hz_thread_pool> userExecutor;
                    spi::LifecycleService &lifecycleService;
                    const ClientProperties &clientProperties;
                    int userExecutorPoolSize;

                    template<typename CompletionToken>
                    std::shared_ptr<boost::asio::steady_timer> scheduleWithRepetitionInternal(CompletionToken token,
                                                                                              const std::chrono::steady_clock::duration &delay,
                                                                                              const std::chrono::steady_clock::duration &period,
                                                                                              std::shared_ptr<boost::asio::steady_timer> timer) {
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
                            } catch (std::exception &) {
                                assert(false);
                            }

                            if (lifecycleService.isRunning() && period.count()) {
                                scheduleWithRepetitionInternal(token, std::chrono::seconds(-1), period, timer);
                            }
                        });
                        return timer;
                    }
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


