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

class MyListener : public hazelcast::client::topic::ReliableMessageListener {
public:
    MyListener(std::atomic<int> &numberOfMessagesReceived, int64_t sequence) : numberOfMessagesReceived(
            numberOfMessagesReceived), startSequence(sequence), lastReceivedSequence(-1) {}

    MyListener(std::atomic<int> &numberOfMessagesReceived) : numberOfMessagesReceived(
            numberOfMessagesReceived), startSequence(-1), lastReceivedSequence(-1) {}

    void onMessage(hazelcast::client::topic::Message &&message) override {
        ++numberOfMessagesReceived;

        auto object = message.getMessageObject().get<std::string>();
        if (object) {
            std::cout << "[GenericListener::onMessage] Received message: " << *object << " for topic:" << message.getName();
        } else {
            std::cout << "[GenericListener::onMessage] Received message with NULL object for topic:" <<
            message.getName();
        }
    }

    int64_t retrieveInitialSequence() const override {
        return startSequence;
    }

    void storeSequence(int64_t sequence) override {
        lastReceivedSequence = sequence;
    }

    bool isLossTolerant() const override {
        return false;
    }

    bool isTerminal(const hazelcast::client::exception::IException &failure) const override {
        return false;
    }

private:
    std::atomic<int> &numberOfMessagesReceived;
    int64_t startSequence;
    int64_t lastReceivedSequence;
};

void listenWithDefaultConfig() {
    hazelcast::client::HazelcastClient client;

    std::string topicName("MyReliableTopic");
    auto topic = client.getReliableTopic(topicName);

    std::atomic<int> numberOfMessagesReceived{0};
    std::string listenerId = topic->addMessageListener(MyListener(numberOfMessagesReceived));

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (numberOfMessagesReceived < 1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (topic->removeMessageListener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}

void listenWithConfig() {
    hazelcast::client::ClientConfig clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.setReadBatchSize(5);
    clientConfig.addReliableTopicConfig(reliableTopicConfig);
    hazelcast::client::HazelcastClient client(clientConfig);

    auto topic = client.getReliableTopic(topicName);

    std::atomic<int> numberOfMessagesReceived{0};
    const std::string &listenerId = topic->addMessageListener(MyListener(numberOfMessagesReceived));

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (numberOfMessagesReceived < 1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (topic->removeMessageListener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}

int main() {
    listenWithDefaultConfig();
    
    listenWithConfig();

    std::cout << "Finished" << std::endl;

    return 0;
}
