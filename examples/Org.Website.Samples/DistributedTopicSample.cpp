#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class DistributedTopicSample {
public:
    void onMessage(topic::Message<std::string> message) {
        std::cout << "Got message " << message.getMessageObject() << std::endl;
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get a Topic called "my-distributed-topic"
    ITopic<std::string> topic = hz.getTopic<std::string>("my-distributed-topic");
    // Add a Listener to the Topic
    DistributedTopicSample listener;
    topic.addMessageListener<DistributedTopicSample>(listener);
    // Publish a message to the Topic
    topic.publish("Hello to distributed world");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}