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

#include <stdint.h>
#include <atomic>

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

namespace serialization {}
namespace spi {
class ClientContext;
}
namespace connection {
class Connection;

class HAZELCAST_API ReadHandler
{
public:
    ReadHandler(Connection& connection, size_t buffer_size);

    ~ReadHandler();

    void handle();

    std::chrono::steady_clock::time_point get_last_read_time() const;

    char* buffer;
    util::ByteBuffer byte_buffer;

private:
    protocol::ClientMessageBuilder<Connection> builder_;
    std::atomic<std::chrono::steady_clock::duration> last_read_time_;
};
} // namespace connection
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
