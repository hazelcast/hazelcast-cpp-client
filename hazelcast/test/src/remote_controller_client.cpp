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

#include "remote_controller_client.h"

#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

namespace hazelcast {
namespace client {
namespace test {

std::string
remote_controller_address()
{
    // TODO this can be read from an environment variable to make it configurable
    return "127.0.0.1";
}

remote::RemoteControllerClient&
remote_controller_client()
{
    using namespace apache::thrift::protocol;
    using namespace apache::thrift::transport;

    static auto socket = std::make_shared<TSocket>(remote_controller_address(), 9701);
    static auto transport = std::make_shared<TBufferedTransport>(socket);
    static auto protocol = std::make_shared<TBinaryProtocol>(transport);

    static remote::RemoteControllerClient client{ protocol };

    transport->open();

    return client;
}

} // namespace test
} // namespace client
} // namespace hazelcast
