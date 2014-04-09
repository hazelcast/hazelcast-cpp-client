//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DataSerializable
#define HAZELCAST_DataSerializable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
            namespace pimpl {
                class HAZELCAST_API DataSerializable {
                public:

                    virtual ~DataSerializable();

                    virtual void writeData(serialization::ObjectDataOutput &writer) const = 0;

                    virtual void readData(serialization::ObjectDataInput &reader) = 0;

                };
            }
        }


    }
}


#endif //HAZELCAST_DataSerializable

