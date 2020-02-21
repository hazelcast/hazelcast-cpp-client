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

class MyTopicListener : public hazelcast::client::topic::MessageListener<std::string> {
public:
    void onMessage(std::unique_ptr<hazelcast::client::topic::Message<std::string> > &&msg) {
        std::cout << "[MyTopicListener::onMessage] Message received:" << msg->getMessageObject() << std::endl;
    }
};

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::ITopic<std::string> topic = hz.getTopic<std::string>("testtopic");
    MyTopicListener listener;
    topic.addMessageListener(listener);
    std::cout << "Subscriber: Added topic listener. Waiting 5 seconds for the published topic." << std::endl;
    hazelcast::util::sleep(5);

    std::cout << "Finished" << std::endl;

    return 0;
}
