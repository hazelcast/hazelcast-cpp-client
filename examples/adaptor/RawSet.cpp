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
#include <hazelcast/client/adaptor/RawPointerSet.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::ISet<std::string> s = hz.getSet<std::string>("set");
    hazelcast::client::adaptor::RawPointerSet<std::string> set(s);
    set.add("Tokyo");
    set.add("Paris");
    set.add("New York");
    std::cout << "Finished loading set" << std::endl;

    std::auto_ptr<hazelcast::client::adaptor::DataArray<std::string> > vals = set.toArray();

    std::cout << "There are " << set.size() << " values in the set" << std::endl;

    for (size_t i = 0; i < vals->size(); ++i) {
        const std::string *val = (*vals)[i];
        if (NULL == val) {
            std::cout << "Value " << i << " is NULL" << std::endl;
        } else {
            std::cout << "Value: " << *val << std::endl;
        }
    }

    bool exists = set.remove("Tokyo");
    if (exists) {
        std::cout << "Removed Tokyo from set" << std::endl;
    } else {
        std::cout << "Could not remove Tokyo from set. It did not exist." << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
