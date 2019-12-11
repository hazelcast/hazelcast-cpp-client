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

#include "AppendCallable.h"
#include "IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>

std::string hazelcast::client::test::executor::tasks::AppendCallable::APPENDAGE = ":CallableResult";

int hazelcast::client::test::executor::tasks::AppendCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::AppendCallable::getClassId() const {
    return IdentifiedFactory::APPEND_CALLABLE;
}

void hazelcast::client::test::executor::tasks::AppendCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&msg);
}

void hazelcast::client::test::executor::tasks::AppendCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // No need to implement this part for the client
}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable() {}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable(const std::string &msg) : msg(msg) {}
