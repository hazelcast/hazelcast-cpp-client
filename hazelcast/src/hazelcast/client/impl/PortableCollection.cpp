//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PortableCollection.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            int PortableCollection::getTypeSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            int PortableCollection::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableCollection::getClassId() const {
                return protocol::SpiConstants::COLLECTION;
            };
        }
    }
}