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
//
// Created by İhsan Demir on 26/05/15.
//

#ifndef HAZELCASTCLIENT_CLIENTTESTSUPPORT_H
#define HAZELCASTCLIENT_CLIENTTESTSUPPORT_H

#include <memory>
#include <boost/thread/latch.hpp>

#include <gtest/gtest.h>

#include "ClientTestSupportBase.h"

#include <hazelcast/util/ILogger.h>

#define assertEquals ASSERT_EQ
#define assertTrue ASSERT_TRUE
#define assertFalse ASSERT_FALSE
#define assertOpenEventually ASSERT_OPEN_EVENTUALLY

namespace hazelcast {
    namespace client {
        class ClientConfig;

        class HazelcastClient;

        namespace test {
            class HazelcastServerFactory;

            extern HazelcastServerFactory *g_srvFactory;

            class ClientTestSupport : public ClientTestSupportBase, public ::testing::Test {
            public:
                ClientTestSupport();

            protected:
                util::ILogger &getLogger();

                const std::string &getTestName() const;

            private:
                std::shared_ptr<hazelcast::util::ILogger> logger;
                std::string testName;
            };

            class CountDownLatchWaiter {
            public:
                CountDownLatchWaiter &add(boost::latch &latch1);

                boost::cv_status wait_for(boost::chrono::steady_clock::duration duration);

                void reset();

            private:
                std::vector<boost::latch *> latches;
            };
        }
    }
}

#endif //HAZELCASTCLIENT_CLIENTTESTSUPPORT_H
