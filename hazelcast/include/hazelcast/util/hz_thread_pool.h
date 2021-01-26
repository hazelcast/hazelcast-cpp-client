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

#include <boost/asio/thread_pool.hpp>
#include <boost/thread/executors/work.hpp>

#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API hz_thread_pool {
        public:
            explicit hz_thread_pool();

            explicit hz_thread_pool(size_t num_threads);

            void close();

            bool closed();

            void submit(boost::executors::work &&closure);

            bool try_executing_one();

            boost::asio::thread_pool::executor_type get_executor() const;

        private:
            std::unique_ptr<boost::asio::thread_pool> pool_;
            std::atomic<bool> closed_{false};
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

