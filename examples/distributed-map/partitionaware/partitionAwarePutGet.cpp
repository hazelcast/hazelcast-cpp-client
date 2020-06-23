/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/PartitionAware.h>

struct PartitionAwareString : public hazelcast::client::PartitionAware<std::string> {
    PartitionAwareString(const std::string &actualKey) : actualKey(actualKey) {}

    const std::string *getPartitionKey() const override {
        return &desiredPartitionString;
    }

    std::string actualKey;
    static const std::string desiredPartitionString;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<PartitionAwareString> : identified_data_serializer {
                static int32_t getFactoryId() noexcept {
                    return 1;
                }

                static int32_t getClassId() noexcept {
                    return 10;
                }

                static void
                writeData(const PartitionAwareString &object, hazelcast::client::serialization::ObjectDataOutput &out) {
                    out.write(object.actualKey);
                }

                static PartitionAwareString readData(hazelcast::client::serialization::ObjectDataInput &in) {
                    return PartitionAwareString{in.read<std::string>()};
                }
            };
        }
    }
}


const std::string PartitionAwareString::desiredPartitionString = "desiredKeyString";

int main() {
    hazelcast::client::HazelcastClient hz;

    PartitionAwareString partitionKey{"MyString"};

    auto map = hz.getMap("paritionawaremap");

    // Puts the key, value tokyo at the partition for "desiredKeyString" rather than the partition for "MyString"
    map->put<PartitionAwareString, std::string>(partitionKey, "Tokyo").get();
    auto value = map->get<PartitionAwareString, std::string>(partitionKey).get();

    std::cout << "Got the value for key " << partitionKey.actualKey << ". PartitionAwareString is:" << value.value()
              << '\n';

    std::cout << "Finished" << std::endl;

    return 0;
}
