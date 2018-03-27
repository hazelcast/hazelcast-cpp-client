#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Set from Cluster.
    ISet<std::string> set = hz.getSet<std::string>("my-distributed-set");
    // Add items to the set with duplicates
    set.add("item1");
    set.add("item1");
    set.add("item2");
    set.add("item2");
    set.add("item2");
    set.add("item3");
    // Get the items. Note that there are no duplicates.
    std::vector<std::string> values = set.toArray();
    for (std::vector<std::string>::const_iterator it=values.begin();it != values.end();++it) {
        std::cout << (*it) << std::endl;
    }
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
