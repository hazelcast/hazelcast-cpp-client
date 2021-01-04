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

#include <chrono>
#include <random>

#include "hazelcast/util/export.h"

namespace hazelcast {
    namespace client {
        namespace config {
            class connection_retry_config;
        }
        namespace connection {
            class wait_strategy {
            public:
                wait_strategy(const config::connection_retry_config &retry_config, logger &log);

                void reset();

                bool sleep();

            private:
                const std::chrono::milliseconds initial_backoff_millis_;
                const std::chrono::milliseconds max_backoff_millis_;
                const double multiplier_;
                const double jitter_;
                logger &logger_;
                std::mt19937 random_generator_{std::random_device{}()};
                std::uniform_real_distribution<double> random_{0.0, 1.0};
                int attempt_ = 0;
                std::chrono::milliseconds current_backoff_millis_{0};
                std::chrono::milliseconds cluster_connect_timeout_millis_{0};
                std::chrono::steady_clock::time_point cluster_connect_attempt_begin_{
                        std::chrono::steady_clock::now()};
            };
        }
    }
}



