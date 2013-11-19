//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/Portable.h"
#include "SerializationConstants.h"

namespace hazelcast {
    namespace client {
        Portable::~Portable() {

        }

        int Portable::getSerializerId() const {
            return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
        }
    }
}