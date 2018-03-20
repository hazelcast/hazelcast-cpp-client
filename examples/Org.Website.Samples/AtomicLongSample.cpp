#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get an Atomic Counter, we'll call it "counter"
    IAtomicLong counter = hz.getIAtomicLong("counter");
    // Add and Get the "counter"
    counter.addAndGet(3); // value is now 3
    // Display the "counter" value
    std::cout << "counter: "+counter.get() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}