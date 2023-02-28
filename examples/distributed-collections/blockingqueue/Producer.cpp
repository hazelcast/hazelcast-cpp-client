/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

int
main()
{
    auto hz = hazelcast::new_client().get();

    auto queue = hz.get_queue("queue").get();

    for (int k = 1; k < 100; k++) {
        queue->put(k).get();
        std::cout << "Producing: " << k << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    queue->put(-1).get();
    std::cout << "Producer Finished!" << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
