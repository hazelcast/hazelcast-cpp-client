//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "SerializationConstants.h"

namespace hazelcast {
    namespace client {
        IdentifiedDataSerializable::~IdentifiedDataSerializable() {

        }

        int IdentifiedDataSerializable::getSerializerId() const {
            return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
        }

    }
}