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

hazelcast::client::topic::ReliableListener make_listener(std::atomic<int> &n_received_messages, int64_t sequence_id = -1) {
    using namespace hazelcast::client::topic;

    return ReliableListener(false, sequence_id)
        .on_received([&n_received_messages](Message &&message){
            ++n_received_messages;

            auto object = message.get_message_object().get<std::string>();
            if (object) {
                std::cout << "[GenericListener::onMessage] Received message: " << *object << " for topic:" << message.get_name();
            } else {
                std::cout << "[GenericListener::onMessage] Received message with NULL object for topic:" <<
                message.get_name();
            }
        });
}

void listen_with_default_config() {
    hazelcast::client::HazelcastClient client;

    std::string topicName("MyReliableTopic");
    auto topic = client.get_reliable_topic(topicName);

    std::atomic<int> numberOfMessagesReceived{0};
    auto listenerId = topic->add_message_listener(make_listener(numberOfMessagesReceived));

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (numberOfMessagesReceived < 1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (topic->remove_message_listener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}

void listen_with_config() {
    hazelcast::client::ClientConfig clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.set_read_batch_size(5);
    clientConfig.add_reliable_topic_config(reliableTopicConfig);
    hazelcast::client::HazelcastClient client(clientConfig);

    auto topic = client.get_reliable_topic(topicName);

    std::atomic<int> numberOfMessagesReceived{0};
    auto listenerId = topic->add_message_listener(make_listener(numberOfMessagesReceived));

    std::cout << "Registered the listener with listener id:" << listenerId << std::endl;

    while (numberOfMessagesReceived < 1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (topic->remove_message_listener(listenerId)) {
        std::cout << "Successfully removed the listener " << listenerId << " for topic " << topicName << std::endl;
    } else {
        std::cerr << "Failed to remove the listener " << listenerId << " for topic " << topicName << std::endl;
    }
}

int main() {
    listen_with_default_config();
    
    listen_with_config();

    std::cout << "Finished" << std::endl;

    return 0;
}
