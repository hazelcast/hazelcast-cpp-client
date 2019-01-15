/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include "SelectNoMembers.h"
#include "IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>

int hazelcast::client::test::executor::tasks::SelectNoMembers::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SelectNoMembers::getClassId() const {
    return IdentifiedFactory::SELECTNOMEMBERS;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::SelectNoMembers::SelectNoMembers() {}

bool hazelcast::client::test::executor::tasks::SelectNoMembers::select(const hazelcast::client::Member &member) const {
    return false;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::toString(std::ostream &os) const {
    os << "SelectNoMembers";
}
