#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get a Blocking Queue called "my-distributed-queue"
    IQueue<std::string> queue = hz.getQueue<std::string>("my-distributed-queue");
    // Offer a String into the Distributed Queue
    queue.offer("item");
    // Poll the Distributed Queue and return the String
    boost::shared_ptr<std::string> item = queue.poll();
    //Timed blocking Operations
    queue.offer("anotheritem", 500);
    boost::shared_ptr<std::string> anotherItem = queue.poll(5 * 1000);
    //Indefinitely blocking Operations
    queue.put("yetanotheritem");
    std::cout << *queue.take() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}