/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>

class MapInterceptor : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    MapInterceptor() { }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 7;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
    }
};

/*
 * For this program, you should first build and run sample server which is provided as
 * MapInterceptorServer in /java/src/main/java/distributedMapInterceptor.
 */
int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, std::string> map =
            hz.getMap<std::string, std::string>("themap");

    MapInterceptor interceptor;
    map.addInterceptor<MapInterceptor>(interceptor);

    map.put("1", "1");

    std::cout << "The modified value (modified by the interceptor) at the server:" << map.get("1") << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
