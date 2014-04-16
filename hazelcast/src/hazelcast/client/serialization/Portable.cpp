//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            Portable::~Portable() {

            }

            int Portable::getSerializerId() const {
                return pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }
        }
    }
}
