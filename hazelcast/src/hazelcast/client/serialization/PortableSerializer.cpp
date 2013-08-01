//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/PortableSerializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableSerializer::PortableSerializer(SerializationContext& serializationContext)
            : context(serializationContext) {
            };

        }
    }
}

