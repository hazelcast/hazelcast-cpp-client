/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/adaptor/RawPointerList.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IList<std::string> l = hz.getList<std::string>("list");
    hazelcast::client::adaptor::RawPointerList<std::string> list(l);

    std::cout << "There are " << list.size() << " values initally in the list" << std::endl;

    list.clear();

    std::cout << "After clear operation, there are " << list.size() << " values in the list" << std::endl;

    list.add("Tokyo");
    list.add("Paris");
    list.add("New York");
    std::cout << "Finished loading list" << std::endl;

    std::cout << "There are " << list.size() << " values in the list" << std::endl;

    std::auto_ptr<hazelcast::client::DataArray<std::string> > vals = list.subList(0, 2);
    std::cout << "Got sublist between indexes 0 and 2. Size is:" << vals->size() << std::endl;
    for (size_t i = 0; i < vals->size(); ++i) {
        const std::string *val = vals->get(i);
        if (NULL == val) {
            std::cout << "Value " << i << " is NULL" << std::endl;
        } else {
            std::cout << "Value: " << *val << std::endl;
        }
    }

    std::auto_ptr<std::string> item = list.get(1);
    if (NULL != item.get()) {
        std::cout << "Item at index 1 is " << *item << std::endl;
    } else {
        std::cout << "Item at index 1 is NULL" << std::endl;
    }

    vals = list.toArray();

    for (size_t i = 0; i < vals->size(); ++i) {
        std::auto_ptr<std::string> val = vals->release(i);
        if (NULL == val.get()) {
            std::cout << "Value " << i << " is NULL" << std::endl;
        } else {
            std::cout << "Value: " << *val << std::endl;
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
