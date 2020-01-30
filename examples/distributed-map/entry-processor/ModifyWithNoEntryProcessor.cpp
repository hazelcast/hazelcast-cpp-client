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

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IMap<std::string, Employee> employees = hz.getMap<std::string, Employee>("employees");

    employees.put("John", Employee(1000));
    employees.put("Mark", Employee(1000));
    employees.put("Spencer", Employee(1000));

    std::vector<std::pair<std::string, Employee> > entries = employees.entrySet();

    for (std::vector<std::pair<std::string, Employee> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        std::shared_ptr<Employee> employee = employees.get(it->first);
        employee->incSalary(10);
        employees.put(it->first, *employee);
    }

    entries = employees.entrySet();

    for (std::vector<std::pair<std::string, Employee> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        std::cout << it->first << " salary: " << it->second.getSalary() << std::endl;
    }


    std::cout << "Finished" << std::endl;

    return 0;
}
