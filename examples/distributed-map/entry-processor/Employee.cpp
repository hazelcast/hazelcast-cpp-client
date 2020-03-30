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
// Created by İhsan Demir on 28/12/15.
//

#include "Employee.h"

#include <hazelcast/client/serialization/ObjectDataInput.h>
#include <hazelcast/client/serialization/ObjectDataOutput.h>

Employee::Employee() {

}

Employee::Employee(int s) : salary(s) {
}

int Employee::getFactoryId() const {
    return 1;
}

int Employee::getClassId() const {
    return 5;
}

void Employee::writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
    out.writeInt(salary);
}

void Employee::readData(hazelcast::client::serialization::ObjectDataInput &in) {
    salary = in.readInt();
}

void Employee::incSalary(int amount) {
    salary += amount;
}

int Employee::getSalary() const {
    return salary;
}
