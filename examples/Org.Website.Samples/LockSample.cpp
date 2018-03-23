#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get a distributed lock called "my-distributed-lock"
    ILock lock = hz.getILock("my-distributed-lock");
    // Now create a lock and execute some guarded code.
    lock.lock();
    try {
        //do something here
    } catch (...) {
            lock.unlock();
    }
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}