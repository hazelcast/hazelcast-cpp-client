/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "employee.h"

int main() {
    auto hz = hazelcast::new_client().get();

    auto employees = hz.get_map("employees").get();

    employees->put("John", employee{1000}).get();
    employees->put("Mark", employee{1000}).get();
    employees->put("Spencer", employee{1000}).get();

    employee_raise_entry_processor processor;
    auto result = employees->execute_on_entries<std::string, int, employee_raise_entry_processor>(
            employee_raise_entry_processor{}).get();

    std::cout << "The result after employees.execute_on_entries call is:" << std::endl;
    for (auto &entry : result) {
        std::cout << entry.first << " salary: " << *entry.second << std::endl;
    }

    result = employees->execute_on_keys<std::string, int, employee_raise_entry_processor>({"John", "Spencer"},
                                                                                     employee_raise_entry_processor{}).get();

    std::cout << "The result after employees.executeOnKeys call is:" << std::endl;
    for (auto &entry : result) {
        std::cout << entry.first << " salary: " << *entry.second << std::endl;
    }

    // use submitToKey api
    auto future = employees->submit_to_key<std::string, int, employee_raise_entry_processor>("Mark", employee_raise_entry_processor{});
    // wait for 1 second
    if (future.wait_for(boost::chrono::seconds(1)) == boost::future_status::ready) {
        std::cout << "Got the result of submitToKey in 1 second for Mark" << " new salary: " << *future.get() << std::endl;
    } else {
        std::cout << "Could not get the result of submitToKey in 1 second for Mark" << std::endl;
    }

    // multiple futures
    std::vector<boost::future<boost::optional<int>>> allFutures;

    // test putting into a vector of futures
    future = employees->submit_to_key<std::string, int, employee_raise_entry_processor>(
            "Mark", processor);
    allFutures.push_back(std::move(future));

    allFutures.push_back(employees->submit_to_key<std::string, int, employee_raise_entry_processor>(
            "John", employee_raise_entry_processor{}));

    boost::wait_for_all(allFutures.begin(), allFutures.end());

    for (auto &f : allFutures) {
        std::cout << "Result:" << *f.get() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
