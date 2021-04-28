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

#include <memory>
#include <boost/thread/latch.hpp>

#include <gtest/gtest.h>

#include <hazelcast/logger.h>

#include "ClientTestSupportBase.h"

#define assertEquals ASSERT_EQ
#define assertTrue ASSERT_TRUE
#define assertFalse ASSERT_FALSE
#define assertOpenEventually ASSERT_OPEN_EVENTUALLY

namespace hazelcast {
    namespace client {
        class client_config;

        class hazelcast_client;

        namespace test {
            class HazelcastServerFactory;

            extern HazelcastServerFactory *g_srvFactory;

            class ClientTestSupport : public ClientTestSupportBase, public ::testing::Test {
            public:
                ClientTestSupport();

            protected:
                logger &get_logger();

                static std::string get_test_name();

            private:
                std::shared_ptr<logger> logger_;
            };

            class CountDownLatchWaiter {
            public:
                CountDownLatchWaiter &add(boost::latch &latch1);

                boost::cv_status wait_for(boost::chrono::steady_clock::duration duration);

                void reset();

            private:
                std::vector<boost::latch *> latches_;
            };
        }
    }
}
