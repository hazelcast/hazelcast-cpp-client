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

#include <hazelcast/client/hazelcast.h>

int
main()
{
    hazelcast::client::client_config config;
    config.set_socket_interceptor(hazelcast::client::socket_interceptor().on_connect(
      [](const hazelcast::client::socket& connected_socket) {
          std::cout << "Connected to remote host " << connected_socket.get_address() << std::endl;
      }));

    auto hz = hazelcast::new_client(std::move(config)).get();

    std::cout << "Finished" << std::endl;

    return 0;
}
