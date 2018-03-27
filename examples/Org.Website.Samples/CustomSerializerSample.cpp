#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class CustomSerializable {
public:
    CustomSerializable(const std::string &value) : value(value) {}

    const std::string &getValue() const {
        return value;
    }

    void setValue(const std::string &value) {
        CustomSerializable::value = value;
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
        CustomSerializable *csObject = static_cast<CustomSerializable *>(object);
        const std::string &value = csObject->getValue();
        int length = (int) value.length();
        std::vector<hazelcast::byte> bytes;
        for (int i = 0; i < length; ++i) {
            bytes.push_back((hazelcast::byte) value[i]);
        }
        out.writeInt((int) length);
        out.write(bytes);
    }

    virtual void *read(serialization::ObjectDataInput &in) {
        int32_t len = in.readInt();
        std::ostringstream value;
        for (int i = 0; i < len; ++i) {
            value << (char) in.readByte();
        }

        return new CustomSerializable(value.str());
    }
};

int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().registerSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new CustomSerializer()));

    HazelcastClient hz(clientConfig);

    IMap<long, CustomSerializable> map = hz.getMap<long, CustomSerializable>("customMap");
    map.put(1L, CustomSerializable("fooooo"));

    return 0;
}
