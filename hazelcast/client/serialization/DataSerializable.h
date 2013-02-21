//
//  DataSerializable.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_SERIALIZALE
#define HAZELCAST_DATA_SERIALIZALE

#include <iostream>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataInput;
            class DataOutput;

            class DataSerializable {
            public:

                virtual void writeData(DataOutput&) const = 0;

                virtual void readData(DataInput&) = 0;

            };

        }
    }
}
#endif /* HAZELCAST_DATA_SERIALIZALE */
