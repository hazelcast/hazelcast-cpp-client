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
// Created by sancar koyunlu on 6/21/13.



#include "hazelcast/client/impl/PortableEntryEvent.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            const serialization::pimpl::Data& PortableEntryEvent::getKey() const {
                return key;
            }

            const serialization::pimpl::Data& PortableEntryEvent::getOldValue() const {
                return oldValue;
            }

            const serialization::pimpl::Data& PortableEntryEvent::getValue() const {
                return value;
            }

            const serialization::pimpl::Data& PortableEntryEvent::getMergingValue() const {
                return mergingValue;
            }

            std::string &PortableEntryEvent::getUuid() const {
                return *uuid;
            }

            EntryEventType PortableEntryEvent::getEventType() const {
                return eventType;
            }

            int PortableEntryEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int PortableEntryEvent::getClassId() const {
                return protocol::SpiConstants::ENTRY_EVENT;
            }


            int PortableEntryEvent::getNumberOfAffectedEntries() const {
                return numberOfAffectedEntries;
            }

            void PortableEntryEvent::readPortable(serialization::PortableReader& reader) {
                eventType = reader.readInt("e");
                uuid = reader.readUTF("u");
                numberOfAffectedEntries = reader.readInt("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key = in.readData();
                value = in.readData();
                oldValue = in.readData();
                mergingValue = in.readData();
            }
        }
    }
}

