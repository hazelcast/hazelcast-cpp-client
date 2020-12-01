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
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/partition_aware.h>

struct PartitionAwareString : public hazelcast::client::partition_aware<std::string> {
    PartitionAwareString(const std::string &actual_key) : actual_key(actual_key) {}

    const std::string *get_partition_key() const override {
        return &desiredPartitionString;
    }

    std::string actual_key;
    static const std::string desiredPartitionString;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<PartitionAwareString> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 10;
                }

                static void
                write_data(const PartitionAwareString &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.actual_key);
                }

                static PartitionAwareString read_data(hazelcast::client::serialization::object_data_input &in) {
                    return PartitionAwareString{in.read<std::string>()};
                }
            };
        }
    }
}


const std::string PartitionAwareString::desiredPartitionString = "desiredKeyString";

int main() {
    hazelcast::client::hazelcast_client hz;

    PartitionAwareString partitionKey{"MyString"};

    auto map = hz.get_map("paritionawaremap").get();

    // Puts the key, value tokyo at the partition for "desiredKeyString" rather than the partition for "MyString"
    map->put<PartitionAwareString, std::string>(partitionKey, "Tokyo").get();
    auto value = map->get<PartitionAwareString, std::string>(partitionKey).get();

    std::cout << "Got the value for key " << partitionKey.actual_key << ". PartitionAwareString is:" << value.value()
              << '\n';

    std::cout << "Finished" << std::endl;

    return 0;
}
