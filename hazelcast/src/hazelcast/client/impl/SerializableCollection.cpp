//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/serialization/BufferedDataOutput.h"
#include "hazelcast/client/serialization/BufferedDataInput.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            SerializableCollection::SerializableCollection() {

            };

            SerializableCollection::~SerializableCollection() {
            };

            int SerializableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int SerializableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            }

            const vector<hazelcast::client::serialization::Data>&  SerializableCollection::getCollection() const {
                return data;
            };


            void SerializableCollection::writeData(serialization::BufferedDataOutput& writer) {
                writer.writeInt(data.size());
                for (int i = 0; i < data.size(); ++i) {
                    data[i].writeData(writer);
                }
            };

            void SerializableCollection::readData(serialization::BufferedDataInput& reader) {
                int size = reader.readInt();
                if (size == -1)
                    return;
                data.resize(size);
                for (int i = 0; i < size; i++) {
                    data[i].readData(reader);
                }
            };

        }
    }
}