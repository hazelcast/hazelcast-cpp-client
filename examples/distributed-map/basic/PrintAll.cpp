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
#include <hazelcast/client/hazelcast_client.h>

std::vector<std::shared_ptr<hazelcast::client::hazelcast_client>> clientPool_;

int main() {
    hazelcast::client::hazelcast_client client = hazelcast::new_client().get();

    clientPool_.emplace_back(std::make_shared<hazelcast::client::hazelcast_client>(std::move(client)));

    clientPool_[0]->get_map("abc").get()->put(1, 1).get();

    std::cout << "Finished" << std::endl;

    return 0;
}
