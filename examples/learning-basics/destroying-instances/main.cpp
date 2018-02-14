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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IQueue<std::string> q1 = hz.getQueue<std::string>("q");
    hazelcast::client::IQueue<std::string> q2 = hz.getQueue<std::string>("q");

    q1.put("foo");
    std::cout << "q1.size:" << q1.size() << "  q2.size:" << q2.size() << std::endl;

    q1.destroy();
    std::cout << "q1.size:" << q1.size() << "  q2.size:" << q2.size() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
