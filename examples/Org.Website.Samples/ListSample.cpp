#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed List from Cluster.
    IList<std::string> list = hz.getList<std::string>("my-distributed-list");
    // Add elements to the list
    list.add("item1");
    list.add("item2");

    // Remove the first element
    std::cout << "Removed: " << *list.remove(0);
    // There is only one element left
    std::cout << "Current size is " << list.size() << std::endl;
    // Clear the list
    list.clear();
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
