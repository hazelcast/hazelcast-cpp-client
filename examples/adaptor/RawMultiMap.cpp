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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/adaptor/RawPointerMultiMap.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::MultiMap<std::string, std::string> m = hz.getMultiMap<std::string, std::string>("multimap");
    hazelcast::client::adaptor::RawPointerMultiMap<std::string, std::string> multimap(m);
    multimap.put("1", "Tokyo");
    multimap.put("2", "Paris");
    multimap.put("3", "New York");
    std::cout << "Finished loading multimap" << std::endl;

    std::unique_ptr<hazelcast::client::DataArray<std::string> > vals = multimap.values();
    std::unique_ptr<hazelcast::client::EntryArray<std::string, std::string> > entries = multimap.entrySet();

    std::cout << "There are " << vals->size() << " values in the multimap" << std::endl;
    size_t size = entries->size();
    std::cout << "There are " << size << " entries in the multimap" << std::endl;
    
    for (size_t i = 0; i < size; ++i) {
        const std::string * key = entries->getKey(i);
        if ((std::string *) NULL == key) {
            std::cout << "The key at index " << i << " is NULL" << std::endl;
        } else {
            const std::string *val = entries->getValue(i);
            std::cout << "(Key, Value) for index " << i << " is: (" << *key << ", " <<
                (val == NULL ? "NULL" : *val) << ")" << std::endl;
        }
    }

    multimap.put("1", "Istanbul");
    std::cout << "Put the second value for key '1' into the multimap" << std::endl;

    vals = multimap.get("1");
    size = vals->size();
    std::cout << "There are " << vals->size() << " values for key '1' in the multimap. These are:" << std::endl;
    for (size_t j = 0; j < size; ++j) {
        const std::string *val = vals->get(j);
        if (NULL == val) {
            std::cout << "NULL" << std::endl;
        } else {
            std::cout << *val << std::endl;
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
