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
#include <hazelcast/client/hazelcast_client.h>

// You should use the config file hazelcast-username-password.xml when starting
// the server so that the server will authenticate the client successfully.
int
main()
{
    hazelcast::client::client_config clientConfig;

    // set the username and password to match the server side config.
    clientConfig.set_credentials(
      std::make_shared<
        hazelcast::client::security::username_password_credentials>(
        "test-user", "test-pass"));

    auto hz = hazelcast::new_client(std::move(clientConfig)).get();

    auto map = hz.get_map("MyMap").get();

    map->put(1, 100).get();

    auto value = map->get<int, int>(1).get();

    if (value) {
        std::cout << "Value for key 1 is " << value.value() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
