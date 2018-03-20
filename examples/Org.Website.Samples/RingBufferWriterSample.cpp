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

    long i = 100;
    while (true) {
        long sleepMs = 100;
        for (;;) {
            long result = rb->add(i);
            if (result != -1) {
                break;
            }
            hazelcast::util::sleepmillis(sleepMs);
            sleepMs = (sleepMs * 2 > 5000) ? 5000 : sleepMs * 2;
        }

        // add a bit of random delay to make it look a bit more realistic
        hazelcast::util::sleep(rand() % 10);

        std::cout << "Written: " << i;
        i++;
    }
}