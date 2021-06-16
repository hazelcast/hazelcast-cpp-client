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

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include <hazelcast/client/member.h>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/logger.h>

#include "TestHelperFunctions.h"

namespace hazelcast {
namespace client {
class client_config;

class hazelcast_client;

namespace test {
class HazelcastServerFactory;

extern HazelcastServerFactory* g_srvFactory;

class ClientTestSupportBase
{
public:
    ClientTestSupportBase();

    static std::string get_ca_file_path();

    static std::string random_map_name();

    static std::string random_string();

    static void sleep_seconds(int32_t seconds);

    static boost::uuids::uuid generate_key_owned_by(spi::ClientContext& context,
                                                    const member& member);

    static hazelcast::client::client_config get_config(bool ssl_enabled = false, bool smart = true);

protected:
    static const std::string get_ssl_cluster_name();

    static hazelcast_client get_new_client();

    static const std::string get_ssl_file_path();
};
} // namespace test
} // namespace client

namespace util {
class ThreadArgs
{
public:
    const void* arg0;
    const void* arg1;
    const void* arg2;
    const void* arg3;
    void (*func)(ThreadArgs&);
};

class StartedThread
{
public:
    StartedThread(const std::string& name,
                  void (*func)(ThreadArgs&),
                  void* arg0 = nullptr,
                  void* arg1 = nullptr,
                  void* arg2 = nullptr,
                  void* arg3 = nullptr);

    StartedThread(void(func)(ThreadArgs&),
                  void* arg0 = nullptr,
                  void* arg1 = nullptr,
                  void* arg2 = nullptr,
                  void* arg3 = nullptr);

    virtual ~StartedThread();

    bool join();

    virtual void run();

    virtual const std::string get_name() const;

private:
    ThreadArgs thread_args_;
    std::string name_;
    std::thread thread_;
    std::shared_ptr<logger> logger_;

    void init(void(func)(ThreadArgs&), void* arg0, void* arg1, void* arg2, void* arg3);
};
} // namespace util
} // namespace hazelcast
