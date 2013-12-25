//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_IdentifiedDataSerializable
#define HAZELCAST_IdentifiedDataSerializable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {



        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
        }

        class HAZELCAST_API IdentifiedDataSerializable {
        public:

            virtual ~IdentifiedDataSerializable();

            virtual int getFactoryId() const = 0;

            virtual int getClassId() const = 0;

            virtual void writeData(serialization::ObjectDataOutput& writer) const = 0;

            virtual void readData(serialization::ObjectDataInput& reader) = 0;

            virtual int getSerializerId() const;

        };
    }
}

#endif //HAZELCAST_DataSerializable
