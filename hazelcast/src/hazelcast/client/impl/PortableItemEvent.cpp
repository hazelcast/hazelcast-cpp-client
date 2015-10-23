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
// Created by sancar koyunlu on 6/25/13.



#include "hazelcast/client/impl/PortableItemEvent.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            const serialization::pimpl::Data& PortableItemEvent::getItem() const {
                return item;
            }

            std::string &PortableItemEvent::getUuid() const {
                return *uuid;
            }

            ItemEventType PortableItemEvent::getEventType() const {
                return eventType;
            }

            int PortableItemEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int PortableItemEvent::getClassId() const {
                return protocol::SpiConstants::ITEM_EVENT;
            }

            void PortableItemEvent::readPortable(serialization::PortableReader& reader) {
                eventType = reader.readInt("e");;
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                item = in.readData();
            }

        }
    }
}
