//
// Created by sancar koyunlu on 6/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TYPE_SERIALIZER
#define HAZELCAST_TYPE_SERIALIZER

#include <iostream>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class BufferedDataOutput;

            class BufferedDataInput;

            class SerializerBase {
            public:
                virtual ~SerializerBase() {
                };

                virtual int getTypeId() const = 0;
            };

            template <typename Serializable>
            class Serializer : public SerializerBase {
            public:
                virtual ~Serializer() {

                };

                virtual void write(BufferedDataOutput &out, const Serializable& object) = 0;

                virtual void read(BufferedDataInput &in, Serializable & object) = 0;

            };

        }
    }
}


#endif //HAZELCAST_TYPE_SERIALIZER
