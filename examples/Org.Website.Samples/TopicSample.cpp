#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class TopicSample {
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
    TopicSample listener;
    topic.addMessageListener<TopicSample>(listener);
    // Publish a message to the Topic
    topic.publish("Hello to distributed world");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
