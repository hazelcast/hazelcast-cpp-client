#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig config;
    HazelcastClient hz(config);

    /**
     * Ringbuffer "rb" is assumed to be already created at the cluster.
     */
    boost::shared_ptr<Ringbuffer<long> > rb = hz.getRingbuffer<long>("rb");
    // we start from the oldest item.
    // if you want to start from the next item, call rb.tailSequence()+1
    int64_t sequence = rb->headSequence();
    std::cout << "Start reading from: " << sequence << std::endl;
    while (true) {
        std::auto_ptr<long> item = rb->readOne(sequence);
        sequence++;
        std::cout << "Read: " << *item << std::endl;
    }

}