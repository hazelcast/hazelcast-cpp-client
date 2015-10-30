/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by İhsan Demir on 26/10/15.
//

#include "serialization/TestSerializationConstants.h"
#include "MultiplierPortableEntryProcessor.h"

#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            MultiplierPortableEntryProcessor::MultiplierPortableEntryProcessor(int multiplier) : multiplier(
                    multiplier) { }

            int MultiplierPortableEntryProcessor::getFactoryId() const {
                return hazelcast::client::test::TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int MultiplierPortableEntryProcessor::getClassId() const {
                return hazelcast::client::test::TestSerializationConstants::TEST_PORTABLE_ENTRY_PROCESSOR;
            }

            void MultiplierPortableEntryProcessor::writePortable(serialization::PortableWriter &writer) const {
                writer.writeInt("m", multiplier);
            }

            void MultiplierPortableEntryProcessor::readPortable(serialization::PortableReader &reader) {
                multiplier = reader.readInt("m");
            }
        }
    }
}


