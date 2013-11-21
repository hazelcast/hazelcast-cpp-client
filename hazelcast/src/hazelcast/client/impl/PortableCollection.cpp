//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {


            PortableCollection::PortableCollection() {

            };

            const std::vector<serialization::Data>& PortableCollection::getCollection() const {
                return collection;
            };

            int PortableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            };

            void PortableCollection::writePortable(serialization::PortableWriter& writer) const {
                writer.writeBoolean("l", true);
                writer.writeInt("s", collection.size());
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                for (int i = 0; i < collection.size(); ++i) {
                    collection[i].writeData(out);
                }

            };

            void PortableCollection::readPortable(serialization::PortableReader& reader) {
                bool isList = reader.readBoolean("l");
                int size = reader.readInt("s");
                if (size < 0)
                    return;
                collection.resize(size);
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                for (int i = 0; i < size; ++i) {
                    serialization::Data data;
                    data.readData(in);
                    collection[i] = data;
                }
            };
        }
    }
}