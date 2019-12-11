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

class ItemListenerImpl : public hazelcast::client::ItemListener<std::string> {
public:

    ItemListenerImpl() : numAdded(0), numRemoved(0) {
    }

    void itemAdded(const hazelcast::client::ItemEvent<std::string> &item) {
        std::cout << "Item added:" << item.getItem() << std::endl;
        ++numAdded;
    }

    void itemRemoved(const hazelcast::client::ItemEvent<std::string> &item) {
        std::cout << "Item removed:" << item.getItem() << std::endl;
        ++numRemoved;
    }


    int getNumAdded() const {
        return numAdded;
    }

    int getNumRemoved() const {
        return numRemoved;
    }

private:
    int numAdded;
    int numRemoved;
};

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IQueue<std::string> queue = hz.getQueue<std::string>("queue");

    ItemListenerImpl listener;
    std::string registrationId = queue.addItemListener(listener, true);

    std::cout << "Registered the listener with registration id:" << registrationId <<
    "Waiting for the listener events!" << std::endl;

    hazelcast::util::sleep(5);

    std::cout << "Received " << listener.getNumAdded() << " items addition and " << listener.getNumRemoved() <<
    " items removal events." << std::endl;

    // unregister the listener
    if (queue.removeItemListener(registrationId)) {
        std::cout << "Removed the item listener with registration id " << registrationId << std::endl;
    } else {
        std::cout << "Failed to remove the item listener with registration id " << registrationId << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
