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

void publish_with_default_config() {
    hazelcast::client::hazelcast_client client;

    auto topic = client.get_reliable_topic("MyReliableTopic");
    topic->publish(std::string("My first message")).get();
}

void publish_with_non_default_config() {
    hazelcast::client::client_config clientConfig;
    std::string topicName("MyReliableTopic");
    hazelcast::client::config::ReliableTopicConfig reliableTopicConfig(topicName.c_str());
    reliableTopicConfig.set_read_batch_size(5);
    clientConfig.add_reliable_topic_config(reliableTopicConfig);
    hazelcast::client::hazelcast_client client(clientConfig);

    auto topic = client.get_reliable_topic(topicName);

    topic->publish(std::string("My first message")).get();
}


int main() {
    publish_with_default_config();

    publish_with_non_default_config();

    std::cout << "Finished" << std::endl;

    return 0;
}

