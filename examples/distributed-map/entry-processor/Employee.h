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

#pragma once
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>

class Employee : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Employee();

    Employee(int s);

    int getFactoryId() const;

    int getClassId() const;

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const;

    void readData(hazelcast::client::serialization::ObjectDataInput &in);

    void incSalary(int amount);

    int getSalary() const;

private:
    int salary;
};


