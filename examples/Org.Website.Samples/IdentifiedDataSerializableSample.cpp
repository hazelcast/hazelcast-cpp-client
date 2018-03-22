#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class Employee;

class SampleDataSerializableFactory : public serialization::DataSerializableFactory {
public:
    static const int FACTORY_ID = 1000;

    virtual std::auto_ptr<serialization::IdentifiedDataSerializable> create(int32_t classId) {
        switch (classId) {
            case 100:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Employee());
            default:
                std::ostringstream out;
                out << "Class id " << classId << " is not found";
                throw exception::HazelcastSerializationException("SampleDataSerializableFactory::create", out.str());
        }

    }
};

class Employee : public serialization::IdentifiedDataSerializable {
public:
    static const int TYPE_ID = 100;

    virtual int getFactoryId() const {
        return SampleDataSerializableFactory::FACTORY_ID;
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
        // assuming that read name is never null, we can de-reference it
        name = *reader.readUTF();
    }

private:
    int id;
    std::string name;
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;

    clientConfig.getSerializationConfig().addDataSerializableFactory(SampleDataSerializableFactory::FACTORY_ID,
                                                                     boost::shared_ptr<serialization::DataSerializableFactory>(
                                                                             new SampleDataSerializableFactory()));

    HazelcastClient hz(clientConfig);

    return 0;
}