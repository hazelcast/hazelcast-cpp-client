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

#include "SerializedCounterCallable.h"
#include "IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>

int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getClassId() const {
    return IdentifiedFactory::SERIALIZEDCOUNTERCALLABLE;
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeInt(counter + 1);
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    counter = reader.readInt() + 1;
}

hazelcast::client::test::executor::tasks::SerializedCounterCallable::SerializedCounterCallable() : counter(0) {}
