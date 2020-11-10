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

#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/item_listener.h>

int main() {
    hazelcast::client::hazelcast_client hz;

    auto queue = hz.get_queue("queue");

    std::atomic<int> numAdded(0);
    std::atomic<int> numRemoved(0);

    hazelcast::client::item_listener listener;
    listener.
        on_added([&numAdded](hazelcast::client::item_event &&event) {
            std::cout << "Item added:" << event.get_item().get<std::string>().value() << std::endl;
            ++numAdded;
        }).
        on_removed([&numRemoved](hazelcast::client::item_event &&event) {
            std::cout << "Item removed:" << event.get_item().get<std::string>().value() << std::endl;
            ++numRemoved;
        });

    auto registrationId = queue->add_item_listener(std::move(listener), true).get();

    std::cout << "Registered the listener with registration id:" << registrationId <<
    "Waiting for the listener events!" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Received " << numAdded << " items addition and " << numRemoved << " items removal events." << std::endl;

    // unregister the listener
    if (queue->remove_item_listener(registrationId).get()) {
        std::cout << "Removed the item listener with registration id " << registrationId << std::endl;
    } else {
        std::cout << "Failed to remove the item listener with registration id " << boost::uuids::to_string(registrationId) << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
