#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig config;
    HazelcastClient hz(config);
    boost::shared_ptr<Ringbuffer<long> > rb = hz.getRingbuffer<long>("rb");
    // add two items into ring buffer
    rb->add(100);
    rb->add(200);
    // we start from the oldest item.
    // if you want to start from the next item, call rb.tailSequence()+1
    int64_t sequence = rb->headSequence();
    std::cout << *rb->readOne(sequence) << std::endl;
    sequence++;
    std::cout << *rb->readOne(sequence) << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

}