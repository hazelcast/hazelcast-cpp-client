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

using namespace hazelcast::client;

class CustomSerializable {
public:
    CustomSerializable() {}

    CustomSerializable(const std::string &value) : value(value) {}

    virtual ~CustomSerializable() {
    }

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
        const CustomSerializable *csObject = static_cast<const CustomSerializable *>(object);
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
            std::shared_ptr<serialization::StreamSerializer>(new CustomSerializer()));

    HazelcastClient hz(clientConfig);

    IMap<long, CustomSerializable> map = hz.getMap<long, CustomSerializable>("customMap");
    map.put(1L, CustomSerializable("fooooo"));

    return 0;
}
