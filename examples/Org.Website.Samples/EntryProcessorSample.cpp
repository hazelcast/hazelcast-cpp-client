/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

class IncEntryProcessor : public serialization::IdentifiedDataSerializable {
public:
    virtual int getFactoryId() const {
        return 66;
    }

    virtual int getClassId() const {
        return 1;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    HazelcastClient hz(clientConfig);
    // Get the Distributed Map from Cluster.
    IMap<std::string, int> map = hz.getMap<std::string, int>("my-distributed-map");
    // Put the integer value of 0 into the Distributed Map
    boost::shared_ptr<int> replacedValue = map.put("key", 0);
    // Run the IncEntryProcessor class on the Hazelcast Cluster Member holding the key called "key"
    boost::shared_ptr<std::string> returnValueFromIncEntryProcessor = map.executeOnKey<std::string, IncEntryProcessor>(
            "key", IncEntryProcessor());
    // Show that the IncEntryProcessor updated the value.
    std::cout << "new value:" << map.get("key");
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
