#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed MultiMap from Cluster.
    MultiMap<std::string, std::string> multiMap = hz.getMultiMap<std::string, std::string>("my-distributed-multimap");
    // Put values in the map against the same key
    multiMap.put("my-key", "value1");
    multiMap.put("my-key", "value2");
    multiMap.put("my-key", "value3");
    // Print out all the values for associated with key called "my-key"
    std::vector<std::string> values = multiMap.get("my-key");
    for (std::vector<std::string>::const_iterator it = values.begin();it != values.end(); ++it) {
        std::cout << *it << std::endl;
    }
    // remove specific key/value pair
    multiMap.remove("my-key", "value2");    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
