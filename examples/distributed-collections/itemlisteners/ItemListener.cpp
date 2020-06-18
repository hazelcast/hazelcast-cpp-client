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

class ItemListenerImpl {
public:
    ItemListenerImpl(std::atomic<int> &numAdded, std::atomic<int> &numRemoved) : numAdded(numAdded),
                                                                                 numRemoved(numRemoved) {}

    void itemAdded(const hazelcast::client::ItemEvent &item) {
        std::cout << "Item added:" << item.getItem().get<std::string>().value() << std::endl;
        ++numAdded;
    }

    void itemRemoved(const hazelcast::client::ItemEvent &item) {
        std::cout << "Item removed:" << item.getItem().get<std::string>().value() << std::endl;
        ++numRemoved;
    }

private:
    std::atomic<int> &numAdded;
    std::atomic<int> &numRemoved;
};

int main() {
    hazelcast::client::HazelcastClient hz;

    auto queue = hz.getQueue("queue");

    std::atomic<int> numAdded(0);
    std::atomic<int> numRemoved(0);

    std::string registrationId = queue->addItemListener(ItemListenerImpl(numAdded, numRemoved), true).get();

    std::cout << "Registered the listener with registration id:" << registrationId <<
    "Waiting for the listener events!" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Received " << numAdded << " items addition and " << numRemoved << " items removal events." << std::endl;

    // unregister the listener
    if (queue->removeItemListener(registrationId).get()) {
        std::cout << "Removed the item listener with registration id " << registrationId << std::endl;
    } else {
        std::cout << "Failed to remove the item listener with registration id " << registrationId << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
