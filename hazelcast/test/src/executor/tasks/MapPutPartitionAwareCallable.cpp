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

#include "MapPutPartitionAwareCallable.h"
#include "IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>

int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getClassId() const {
    return IdentifiedFactory::MAP_PUTPARTITIONAWARE_CALLABLE;
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&mapName);
    writer.writeObject<std::string>(&partitionKey);
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // no need to implement at client side for the tests
}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable() {}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable(
        const std::string &mapName, const std::string &partitionKey) : mapName(mapName), partitionKey(partitionKey) {}

const std::string *hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getPartitionKey() const {
    return &partitionKey;
}
