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
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    auto queue = hz.getQueue("queue");

    while (true) {
        auto item = queue->take<int32_t>().get();
        if (item) {
            std::cout << "Consumed: " << item.value() << std::endl;

            if (item.value() == -1) {
                queue->put(-1).get();
                break;
            }
        } else {
            std::cout << "Retrieved item is null." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    std::cout << "Consumer Finished!" << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
