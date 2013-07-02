//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_DataSerializable
#define HAZELCAST_DataSerializable

#include "SerializationConstants.h"
#include "ConstantSerializers.h"

namespace hazelcast {
    namespace client {

        class DataSerializable {
        public:
            virtual ~DataSerializable() {

            };

            virtual int getSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
            }
        };
    }
}

#endif //HAZELCAST_DataSerializable
