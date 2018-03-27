#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class GlobalSerializer : public serialization::StreamSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 20;
    }

    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
        // out.write(MyFavoriteSerializer.serialize(object))
    }

    virtual void *read(serialization::ObjectDataInput &in) {
        // return MyFavoriteSerializer.deserialize(in);
        return NULL;
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().setGlobalSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new GlobalSerializer()));

    HazelcastClient hz(clientConfig);

    return 0;
}
