/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

    hazelcast::client::ISet<std::string> set = hz.getSet<std::string>("set");

    std::vector<std::string> listValues = set.toArray();

    for (std::vector<std::string>::const_iterator it = listValues.begin(); it != listValues.end(); ++it) {
        std::cout << *it << std::endl;
    }

    std::cout << "Reading finished!" << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
