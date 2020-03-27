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
// Created by İhsan Demir on 21/12/15.
//
/**
 * You need to provide compile flag -DHZ_BUILD_WITH_SSL when compiling.
 */
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::Address serverAddress("127.0.0.1", 5701);
    config.addAddress(serverAddress);

    config.getNetworkConfig().getSSLConfig().
            setEnabled(true).          // Mandatory setting
            addVerifyFile("MyCAFile"). // Mandatory setting
            setCipherList("HIGH");     // optional setting (values for string are described at
                                       // https://www.openssl.org/docs/man1.0.2/apps/ciphers.html)
    
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");
    
    map.put(1, 100);
    map.put(2, 200);

    std::shared_ptr<int> value = map.get(1);

    if (value.get()) {
        std::cout << "Value for key 1 is " << *value << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
