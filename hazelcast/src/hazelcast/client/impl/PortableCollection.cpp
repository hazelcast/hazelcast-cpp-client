//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


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
                    serialization::pimpl::Data data;
                    data.readData(in);
                    collection[i] = data;
                }
            }
        }
    }
}
