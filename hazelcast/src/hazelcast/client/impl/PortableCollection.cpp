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
// Created by sancar koyunlu on 6/12/13.



#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            const std::vector<serialization::pimpl::Data>& PortableCollection::getCollection() const {
                return collection;
            }

            int PortableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int PortableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            }

            void PortableCollection::readPortable(serialization::PortableReader& reader) {
                reader.readBoolean("l");
                int size = reader.readInt("s");
                if (size < 0)
                    return;
                collection.resize(size);
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                for (int i = 0; i < size; ++i) {
                    collection[i] = in.readData();
                }
            }
        }
    }
}
