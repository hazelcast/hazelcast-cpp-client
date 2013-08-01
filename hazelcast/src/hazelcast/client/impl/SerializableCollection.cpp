//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/SerializableCollection.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            SerializableCollection::SerializableCollection() {

            };

            SerializableCollection::~SerializableCollection() {
                for (int i = 0; i < dataCollection.size(); i++) {
                    delete dataCollection[i];
                }
            };

            int SerializableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int SerializableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            }

            const vector<hazelcast::client::serialization::Data *>&  SerializableCollection::getCollection() const {
                return dataCollection;
            };


            void SerializableCollection::writeData(serialization::ObjectDataOutput& writer) {
                writer.writeInt(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); ++i) {
                    dataCollection[i]->writeData(writer);
                }
            };

            void SerializableCollection::readData(serialization::ObjectDataInput& reader) {
                int size = reader.readInt();
                if (size == -1)
                    return;
                for (int i = 0; i < size; i++) {
                    serialization::Data *data = new serialization::Data();
                    data->readData(reader);
                    dataCollection.push_back(data);
                }
            };

        }
    }
}