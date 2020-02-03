/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

    virtual std::unique_ptr<serialization::Portable> create(int32_t classId) const {
        switch (classId) {
            case 1:
                return std::unique_ptr<serialization::Portable>(new PortableSerializableSample());
            default:
                return std::unique_ptr<serialization::Portable>();
        }
    }
};


int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addPortableFactory(SamplePortableFactory::FACTORY_ID,
                                                             std::shared_ptr<serialization::PortableFactory>(
                                                                     new SamplePortableFactory()));

    HazelcastClient hz(clientConfig);
    //Customer can be used here
    hz.shutdown();

    return 0;
}
