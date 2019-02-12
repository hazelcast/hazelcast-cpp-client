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

#include "FailingCallable.h"
#include "IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>

int hazelcast::client::test::executor::tasks::FailingCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::FailingCallable::getClassId() const {
    return IdentifiedFactory::FAILING_CALLABLE;
}

void hazelcast::client::test::executor::tasks::FailingCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::FailingCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::FailingCallable::FailingCallable() {}
