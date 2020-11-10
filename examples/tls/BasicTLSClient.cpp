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
/**
 * You need to provide compile flag -DHZ_BUILD_WITH_SSL when compiling.
 */
#include <hazelcast/client/hazelcast_client.h>

int main() {
    hazelcast::client::client_config config;
    hazelcast::client::address serverAddress("127.0.0.1", 5701);
    config.get_network_config().add_address(serverAddress);

    config.get_network_config().get_ssl_config().
            set_enabled(true).          // Mandatory setting
            add_verify_file("MyCAFile"). // Mandatory setting
            set_cipher_list("HIGH");     // optional setting (values for string are described at
                                       // https://www.openssl.org/docs/man1.0.2/apps/ciphers.html)
    
    hazelcast::client::hazelcast_client hz(config);

    auto map = hz.get_map("MyMap");
    
    map->put(1, 100).get();
    map->put(2, 200).get();

    auto value = map->get<int, int>(1).get();

    if (value) {
        std::cout << "Value for key 1 is " << value.value() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
