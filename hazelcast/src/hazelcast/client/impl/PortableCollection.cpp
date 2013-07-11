//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableCollection.h"

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
        }
    }
}