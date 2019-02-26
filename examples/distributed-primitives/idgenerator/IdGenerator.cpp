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

 /**
  * Pleasee NOTE that this id generator usage is deprecated in favor of using Flake Id Generator.
  * See FlakeIdGenerator.cpp example.
  */

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/IdGenerator.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IdGenerator generator = hz.getIdGenerator("idGenerator");
    for (int i = 0; i < 10000; ++i) {
        hazelcast::util::sleep(1);
        std::cout << "Id : " << generator.newId() << std::endl;
    }


    std::cout << "Finished" << std::endl;

    return 0;
}
