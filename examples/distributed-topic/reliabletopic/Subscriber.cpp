/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by İhsan Demir on 10 June 2016.
//
#include <hazelcast/client/HazelcastClient.h>

class MyListener : public hazelcast::client::topic::ReliableMessageListener<std::string> {
public:
    MyListener() : startSequence(-1), numberOfMessagesReceived(0), lastReceivedSequence(-1) {
    }

    MyListener(int64_t sequence) : startSequence(sequence), numberOfMessagesReceived(0), lastReceivedSequence(-1) {
    }

    virtual ~MyListener() {
    }

    virtual void onMessage(std::auto_ptr<hazelcast::client::topic::Message<std::string> > message) {
        ++numberOfMessagesReceived;

        const std::string *object = message->getMessageObject();
        if (NULL != object) {
            std::cout << "[GenericListener::onMessage] Received message: " << *message->getMessageObject() <<
            " for topic:" << message->getName();
        } else {
            std::cout << "[GenericListener::onMessage] Received message with NULL object for topic:" <<
            message->getName();
        }
    }

    virtual int64_t retrieveInitialSequence() const {
        return startSequence;
    }

    virtual void storeSequence(int64_t sequence) {
        lastReceivedSequence = sequence;
    }

    virtual bool isLossTolerant() const {
        return false;
    }

    virtual bool isTerminal(const hazelcast::client::exception::IException &failure) const {
        return false;
    }

    int getNumberOfMessagesReceived() {
        int value = numberOfMessagesReceived;
        return value;
    }

private:
    int64_t startSequence;
    hazelcast::util::AtomicInt numberOfMessagesReceived;
    int64_t lastReceivedSequence;
};

void listenWithDefaultConfig() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient client(config);

    std::string topicName("MyReliableTopic");
    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);
    
    MyListener listener;
    const std::string &listenerId = topic->addMessageListener(listener);

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (listener.getNumberOfMessagesReceived() < 1) {
        hazelcast::util::sleep(1);
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

    boost::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);

    MyListener listener;
    const std::string &listenerId = topic->addMessageListener(listener);

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (listener.getNumberOfMessagesReceived() < 1) {
        hazelcast::util::sleep(1);
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
