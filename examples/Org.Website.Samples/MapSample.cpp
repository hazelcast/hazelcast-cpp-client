#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Map from Cluster.
    IMap<std::string, std::string> map = hz.getMap<std::string, std::string>("my-distributed-map");
    //Standard Put and Get.
    map.put("key", "value");
    map.get("key");
    //Concurrent Map methods, optimistic updating
    map.putIfAbsent("somekey", "somevalue");
    map.replace("key", "value", "newvalue");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}