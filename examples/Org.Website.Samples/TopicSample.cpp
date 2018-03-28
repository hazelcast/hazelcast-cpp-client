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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class TopicSample : public topic::MessageListener<std::string> {
public:
    virtual void onMessage(std::auto_ptr<topic::Message<std::string> > message) {
        std::cout << "Got message " << message->getMessageObject() << std::endl;
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get a Topic called "my-distributed-topic"
    ITopic<std::string> topic = hz.getTopic<std::string>("my-distributed-topic");
    // Add a Listener to the Topic
    TopicSample listener;
    topic.addMessageListener<TopicSample>(listener);
    // Publish a message to the Topic
    topic.publish("Hello to distributed world");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
