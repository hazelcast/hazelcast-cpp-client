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
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, Employee> employees = hz.getMap<std::string, Employee>("employees");

    employees.put("John", Employee(1000));
    employees.put("Mark", Employee(1000));
    employees.put("Spencer", Employee(1000));

    EmployeeRaiseEntryProcessor processor;
    std::map<std::string, boost::shared_ptr<int> > result =
            employees.executeOnEntries<int, EmployeeRaiseEntryProcessor>(processor);

    std::cout << "The result after employees.executeOnEntries call is:" << std::endl;
    for (std::map<std::string, boost::shared_ptr<int> >::const_iterator it = result.begin(); it != result.end(); ++it) {
        std::cout << it->first << " salary: " << *it->second << std::endl;
    }

    std::set<std::string> keys;
    keys.insert("John");
    keys.insert("Spencer");

    result = employees.executeOnKeys<int, EmployeeRaiseEntryProcessor>(keys, processor);

    std::cout << "The result after employees.executeOnKeys call is:" << std::endl;
    for (std::map<std::string, boost::shared_ptr<int> >::const_iterator it = result.begin(); it != result.end(); ++it) {
        std::cout << it->first << " salary: " << *it->second << std::endl;
    }
    
    // use submitToKey api
    hazelcast::client::Future<int> future = employees.submitToKey<int, EmployeeRaiseEntryProcessor>("Mark", processor);
    // wait for 1 second
    if (future.wait_for(1000) == hazelcast::client::future_status::ready) {
        std::auto_ptr<int> result = future.get();
        std::cout << "Got the result of submitToKey in 1 second for Mark" << " new salary: " << *result << std::endl;
    } else {
        std::cout << "Could not get the result of submitToKey in 1 second for Mark" << std::endl;
    }

    // multiple futures
    std::vector<hazelcast::client::Future<int> > allFutures;

    // test putting into a vector of futures
    future = employees.submitToKey<int, EmployeeRaiseEntryProcessor>(
            "Mark", processor);
    allFutures.push_back(future);

    allFutures.push_back(employees.submitToKey<int, EmployeeRaiseEntryProcessor>(
            "John", processor));

    for (std::vector<hazelcast::client::Future<int> >::const_iterator it = allFutures.begin();it != allFutures.end();++it) {
        hazelcast::client::future_status status = (*it).wait_for(1000);
        if (status == hazelcast::client::future_status::ready) {
            std::cout << "Got ready for the future" << std::endl;
        }
    }

    for (std::vector<hazelcast::client::Future<int> >::iterator it = allFutures.begin();it != allFutures.end();++it) {
        std::auto_ptr<int> result = (*it).get();
        std::cout << "Result:" << *result << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
