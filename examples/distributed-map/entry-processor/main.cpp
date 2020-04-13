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

#include "Employee.h"

class EmployeeRaiseEntryProcessor : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 6;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &writer) const {
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &reader) {
    }
};


int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IMap<std::string, Employee> employees = hz.getMap<std::string, Employee>("employees");

    employees.put("John", Employee(1000));
    employees.put("Mark", Employee(1000));
    employees.put("Spencer", Employee(1000));

    EmployeeRaiseEntryProcessor processor;
    std::map<std::string, std::shared_ptr<int> > result =
            employees.executeOnEntries<int, EmployeeRaiseEntryProcessor>(processor);

    std::cout << "The result after employees.executeOnEntries call is:" << std::endl;
    for (std::map<std::string, std::shared_ptr<int> >::const_iterator it = result.begin(); it != result.end(); ++it) {
        std::cout << it->first << " salary: " << *it->second << std::endl;
    }

    std::set<std::string> keys;
    keys.insert("John");
    keys.insert("Spencer");

    result = employees.executeOnKeys<int, EmployeeRaiseEntryProcessor>(keys, processor);

    std::cout << "The result after employees.executeOnKeys call is:" << std::endl;
    for (std::map<std::string, std::shared_ptr<int> >::const_iterator it = result.begin(); it != result.end(); ++it) {
        std::cout << it->first << " salary: " << *it->second << std::endl;
    }

    // use submitToKey api
    boost::future<std::shared_ptr<int>> future = employees.submitToKey<int, EmployeeRaiseEntryProcessor>("Mark",
                                                                                                         processor);
    // wait for 1 second
    if (future.wait_for(boost::chrono::seconds(1)) == boost::future_status::ready) {
        auto r = future.get();
        std::cout << "Got the result of submitToKey in 1 second for Mark" << " new salary: " << *r << std::endl;
    } else {
        std::cout << "Could not get the result of submitToKey in 1 second for Mark" << std::endl;
    }

    // multiple futures
    std::vector<boost::future<std::shared_ptr<int>>> allFutures;

    // test putting into a vector of futures
    future = employees.submitToKey<int, EmployeeRaiseEntryProcessor>(
            "Mark", processor);
    allFutures.push_back(std::move(future));

    allFutures.push_back(employees.submitToKey<int, EmployeeRaiseEntryProcessor>(
            "John", processor));

    for (auto &f : allFutures) {
        boost::future_status status = f.wait_for(boost::chrono::seconds(1));
        if (status == boost::future_status::ready) {
            std::cout << "Got ready for the future" << std::endl;
        }
    }

    for (auto &f : allFutures) {
        auto r = f.get();
        std::cout << "Result:" << *r << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
