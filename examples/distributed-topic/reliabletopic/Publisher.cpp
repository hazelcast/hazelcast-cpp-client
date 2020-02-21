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
// Created by İhsan Demir on 10 June 2016.
//
#include <hazelcast/client/HazelcastClient.h>

void publishWithDefaultConfig() {
    hazelcast::client::HazelcastClient client;

    std::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>("MyReliableTopic");
    std::string message("My first message");
    topic->publish(&message);
}

void publishWithNonDefaultConfig() {
    hazelcast::client::ClientConfig clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.setReadBatchSize(5);
    clientConfig.addReliableTopicConfig(reliableTopicConfig);
    hazelcast::client::HazelcastClient client(clientConfig);

    std::shared_ptr<hazelcast::client::ReliableTopic<std::string> > topic = client.getReliableTopic<std::string>(topicName);

    std::string message("My first message");

    topic->publish(&message);
}


int main() {
    publishWithDefaultConfig();

    publishWithNonDefaultConfig();

    std::cout << "Finished" << std::endl;

    return 0;
}

