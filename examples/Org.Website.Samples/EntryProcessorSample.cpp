#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class IncEntryProcessor : public serialization::IdentifiedDataSerializable {
public:
    virtual int getFactoryId() const {
        return 66;
    }

    virtual int getClassId() const {
        return 1;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Map from Cluster.
    IMap<std::string, int> map = hz.getMap<std::string, int>("my-distributed-map");
    // Put the integer value of 0 into the Distributed Map
    boost::shared_ptr<int> replacedValue = map.put("key", 0);
    // Run the IncEntryProcessor class on the Hazelcast Cluster Member holding the key called "key"
    IncEntryProcessor processor;
    // Assume that entry processor returns a string for the sake of example
    boost::shared_ptr<std::string> returnValueFromIncEntryProcessor = map.executeOnKey<std::string, IncEntryProcessor>(
            "key", processor);
    // Show that the IncEntryProcessor updated the value.
    std::cout << "new value:" << map.get("key");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}