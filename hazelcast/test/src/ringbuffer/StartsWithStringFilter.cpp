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

#include "StartsWithStringFilter.h"
#include "IdentifiedFactory.h"

#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                StartsWithStringFilter::StartsWithStringFilter(const std::string &startString) : startString(
                        startString) {}

                int StartsWithStringFilter::getFactoryId() const {
                    return IdentifiedFactory::FACTORY_ID;
                }

                int StartsWithStringFilter::getClassId() const {
                    return IdentifiedFactory::STARTS_WITH_STRING_FILTER;
                }

                void StartsWithStringFilter::writeData(serialization::ObjectDataOutput &writer) const {
                    writer.writeUTF(&startString);
                }

                void StartsWithStringFilter::readData(serialization::ObjectDataInput &reader) {
                    startString = *reader.readUTF();
                }
            }
        }
    }
}