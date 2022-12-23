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

#include "remote_controller_client.h"

#include <string>

#include <boost/algorithm/string.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

namespace hazelcast {
namespace client {
namespace test {

std::string
remote_controller_address()
{
    // TODO this can be read from an environment variable to make it
    // configurable
    return "127.0.0.1";
}

remote::RemoteControllerClient&
remote_controller_client()
{
    using namespace apache::thrift::protocol;
    using namespace apache::thrift::transport;

    static auto socket =
      std::make_shared<TSocket>(remote_controller_address(), 9701);
    static auto transport = std::make_shared<TBufferedTransport>(socket);
    static auto protocol = std::make_shared<TBinaryProtocol>(transport);

    static remote::RemoteControllerClient client{ protocol };

    // if this is the first time this function is called,
    // open the transport.
    if (!transport->isOpen()) {
        transport->open();
    }

    return client;
}

member::version
cluster_version()
{
    using namespace remote;

    Response response;

    remote_controller_client().executeOnController(
      response,
      std::string{},
      "result=com.hazelcast.instance.GeneratedBuildProperties.VERSION;",
      Lang::type::JAVASCRIPT);

    std::vector<std::string> major_minor_patch;

    boost::split(major_minor_patch, response.result, boost::is_any_of("."));

    member::version version;

    version.major = std::stoi(major_minor_patch.at(0));
    version.minor = std::stoi(major_minor_patch.at(1));
    version.patch = std::stoi(major_minor_patch.at(2));

    return version;
}

} // namespace test
} // namespace client
} // namespace hazelcast
