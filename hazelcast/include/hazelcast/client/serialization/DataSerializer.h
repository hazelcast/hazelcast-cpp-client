//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_DATA_SERIALIZER
#define HAZELCAST_DATA_SERIALIZER


namespace hazelcast {
    namespace client {

        class IdentifiedDataSerializable;

        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;


            class DataSerializer {
            public:
                DataSerializer();

                void write(ObjectDataOutput &out, const IdentifiedDataSerializable& object);

                void read(ObjectDataInput& in, IdentifiedDataSerializable& object);

            };
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER
