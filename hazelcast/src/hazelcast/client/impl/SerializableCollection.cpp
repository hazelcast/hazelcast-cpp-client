//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "SerializableCollection.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            SerializableCollection::SerializableCollection() {

            };

            SerializableCollection::~SerializableCollection() {
                vector<hazelcast::client::serialization::Data *>::iterator it;
                for (it = datas.begin(); it != datas.end(); ++it) {
                    delete (*it);
                }
            };

            int SerializableCollection::getSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            };

            int SerializableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            }

            int SerializableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            }

            vector<hazelcast::client::serialization::Data *>  SerializableCollection::getCollection() const {
                return datas;
            };
        }
    }
}