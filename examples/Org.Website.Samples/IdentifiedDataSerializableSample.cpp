#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class SampleDataSerializableFactory;

class Employee : public serialization::IdentifiedDataSerializable {
public:
    static const int TYPE_ID = 100;

    virtual int getFactoryId() const {
        return 1000;
    }

    virtual int getClassId() const {
        return TYPE_ID;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeInt(id);
        writer.writeUTF(&name);
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        id = reader.readInt();
        name = *reader.readUTF();
    }

private:
    int id;
    std::string name;
};

class SampleDataSerializableFactory : public serialization::DataSerializableFactory {
public:
    static const int FACTORY_ID = 1000;

    virtual std::auto_ptr<serialization::IdentifiedDataSerializable> create(int32_t classId) {
        switch (classId) {
            case 100:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Employee());
            default:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>();
        }

    }
};

int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addDataSerializableFactory(SampleDataSerializableFactory::FACTORY_ID,
                                                                     boost::shared_ptr<serialization::DataSerializableFactory>(
                                                                             new SampleDataSerializableFactory()));

    HazelcastClient hz(clientConfig);
    //Employee can be used here
    hz.shutdown();

    return 0;
}
