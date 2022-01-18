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

#include <chrono>

namespace hazelcast {
class logger;

namespace client {
namespace spi {
class ClientContext;
}
namespace connection {
class ClientConnectionManagerImpl;

class Connection;

/**
 * HeartbeatManager manager used by connection manager.
 */
class HeartbeatManager
{
public:
    explicit HeartbeatManager(spi::ClientContext& client,
                              ClientConnectionManagerImpl& connection_manager);

    void start();

    void shutdown();

    std::chrono::milliseconds get_heartbeat_timeout() const;

private:
    spi::ClientContext& client_;
    ClientConnectionManagerImpl& client_connection_manager_;
    logger& logger_;
    std::chrono::milliseconds heartbeat_interval_;
    std::chrono::milliseconds heartbeat_timeout_;
    std::shared_ptr<boost::asio::steady_timer> timer_;

    void check_connection(const std::shared_ptr<Connection>& connection);

    static void on_heartbeat_stopped(
      const std::shared_ptr<Connection>& connection,
      const std::string& reason);
};
} // namespace connection
} // namespace client
} // namespace hazelcast
