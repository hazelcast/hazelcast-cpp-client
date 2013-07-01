//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Portable
#define HAZELCAST_Portable

#include "SerializationConstants.h"

namespace hazelcast{
    namespace client{
        class Portable {
            public:
            virtual ~Portable(){

            };

            virtual int getSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }
        };
    }

}


#endif //HAZELCAST_Portable
