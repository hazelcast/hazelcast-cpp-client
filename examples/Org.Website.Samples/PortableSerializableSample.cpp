#include <hazelcast/client/HazelcastAll.h>
#include <hazelcast/client/serialization/PortableReader.h>
#include <hazelcast/client/serialization/PortableWriter.h>

using namespace hazelcast::client;

class PortableSerializableSample : public serialization::Portable {
public:
    static const int CLASS_ID = 1;

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return CLASS_ID;
    }

    virtual void writePortable(serialization::PortableWriter &writer) const {
        writer.writeInt("id", id);
        writer.writeUTF("name", &name);
        writer.writeLong("lastOrder", lastOrder);
    }

    virtual void readPortable(serialization::PortableReader &reader) {
        id = reader.readInt("id");
        name = *reader.readUTF("name");
        lastOrder = reader.readLong("lastOrder");
    }

private:
    std::string name;
    int32_t id;
    int64_t lastOrder;
};

class SamplePortableFactory : public serialization::PortableFactory {
public:
    static const int FACTORY_ID = 1;

    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
        switch (classId) {
            case 1:
                return std::auto_ptr<serialization::Portable>(new PortableSerializableSample());
            default:
                return std::auto_ptr<serialization::Portable>();
        }
    }
};


int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addPortableFactory(SamplePortableFactory::FACTORY_ID,
                                                             boost::shared_ptr<serialization::PortableFactory>(
                                                                     new SamplePortableFactory()));

    HazelcastClient hz(clientConfig);
    //Customer can be used here
    hz.shutdown();

    return 0;
}
