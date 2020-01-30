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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include <hazelcast/client/serialization/ObjectDataOutput.h>

namespace test1 {
    namespace test2 {
        class Person {
        public:
            Person() {
            }

            Person(const std::string& n) : name(n) {
            }

            void setName(const std::string& n) {
                name = n;
            }

            const std::string& getName() const {
                return name;
            }

        private:
            std::string name;
        };

        /**
         * Put the free function in the same namespace as the object it operates on
         */
        int getHazelcastTypeId(const Person* p){
            return 666;
        }
    }
}

class CustomSerializer : public hazelcast::client::serialization::Serializer<test1::test2::Person> {
public:

    void write(hazelcast::client::serialization::ObjectDataOutput & out, const test1::test2::Person& object) {
        out.writeInt(666);
        out.writeUTF(&(object.getName()));
        out.writeInt(666);
    }

    void read(hazelcast::client::serialization::ObjectDataInput & in, test1::test2::Person& object) {
        int i = in.readInt();
        assert(i == 666);
        object.setName(*(in.readUTF()));
        i = in.readInt();
        assert(i == 666);
    }

    int getHazelcastTypeId() const {
        return 666;
    };
};

std::ostream &operator<<(std::ostream &out, const test1::test2::Person &p) {
    const std::string & str = p.getName();
    out << str;
    return out;
}

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::SerializationConfig serializationConfig;
    serializationConfig.registerSerializer(std::shared_ptr<hazelcast::client::serialization::SerializerBase>(
            new CustomSerializer()));
    config.setSerializationConfig(serializationConfig);
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, test1::test2::Person> map = hz.getMap<std::string, test1::test2::Person>("map");
    test1::test2::Person testPerson("bar");
    map.put("foo", testPerson);
    std::cout << *(map.get("foo")) << std::endl;
    std::cout << "Finished" << std::endl;

    return 0;
}

