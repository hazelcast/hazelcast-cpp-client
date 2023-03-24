/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/client/socket.h"
#include "hazelcast/client/connection/Connection.h"

namespace hazelcast {
namespace client {
namespace connection {

class tpc_channel_connector
  : public std::enable_shared_from_this<tpc_channel_connector>
{
public:
    tpc_channel_connector(boost::uuids::uuid client_uuid,
                          std::shared_ptr<connection::Connection> conn,
                          std::vector<int> ports,
                          hazelcast::util::hz_thread_pool& executor,
                          internal::socket::SocketFactory& channel_creator,
                          logger&,
                          std::chrono::milliseconds connection_timeout_millis);

    /**
     * Initiates the connection attempts.
     * <p>
     * This call does not block.
     */
    void initiate();
    void connect(std::string host, int port, int index);

private:
    bool connection_failed() const;
    void write_authentication_bytes(socket&);
    void on_successful_channel_connection(std::unique_ptr<socket>, int index);
    void on_failure(std::unique_ptr<socket>);
    void close_channel(std::unique_ptr<socket>);
    void close_all_channels();

    boost::uuids::uuid client_uuid_;
    std::shared_ptr<connection::Connection> connection_;
    std::vector<int> tpc_ports_;
    hazelcast::util::hz_thread_pool& executor_;
    internal::socket::SocketFactory& channel_creator_;
    logger& logger_;
    std::vector<std::unique_ptr<socket>> tpc_channels_;
    std::atomic<int> remaining_;
    std::atomic<bool> failed_;

    std::chrono::milliseconds connection_timeout_millis_;
    std::recursive_mutex mtx_;
};

} // namespace connection
} // namespace client
} // namespace hazelcast