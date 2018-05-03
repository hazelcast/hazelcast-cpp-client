/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::ClientConfig config;
    config.addAddress(hazelcast::client::Address("127.0.0.1", 5701)).addAddress(
            hazelcast::client::Address("127.0.0.1", 5702)).addAddress(
            hazelcast::client::Address("127.0.0.1", 9090)).addAddress(hazelcast::client::Address("127.0.0.1", 9091));

    /**
     * Client shuffles the given member list to prevent all clients to connect to the same node when
     * this property is set to true. When it is set to false, the client tries to connect to the nodes
     * in the given order.
     *
     * We force the client to not shuffle and try connect in the provided order the addresses are added.
     */
    config.setProperty("hazelcast.client.shuffle.member.list", "false");

    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, std::string> map = hz.getMap<int, std::string>("test map");

    std::cout << "Finished" << std::endl;

    return 0;
}

