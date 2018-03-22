#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class CustomSerializableType {
public:
    CustomSerializableType(const std::string &value) : value(value) {}

    const std::string &getValue() const {
        return value;
    }

    void setValue(const std::string &value) {
        CustomSerializableType::value = value;
    }

private:
    std::string value;
};

class CustomSerializer : public serialization::StreamSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 10;
    }

    virtual void write(serialization::ObjectDataOutput &out, const void *object) {

    }

    virtual void *read(serialization::ObjectDataInput &in) {
        int32_t len = in.readInt();
        std::ostringstream value;
        for (int i = 0; i < len; ++i) {
            value << in.readChar();
        }

        return new CustomSerializableType(value.str());
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().registerSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new CustomSerializer()));

    HazelcastClient hz(clientConfig);

    return 0;
}