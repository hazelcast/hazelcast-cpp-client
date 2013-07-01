//
// Created by sancar koyunlu on 6/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TYPE_SERIALIZER
#define HAZELCAST_TYPE_SERIALIZER

namespace hazelcast {
    namespace client {
        namespace serialization {

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

                virtual void write(BufferedDataOutput &out, const Serializable *object) = 0;

                virtual Serializable *read(BufferedDataInput &in) = 0;

            };

        }
    }
}


#endif //HAZELCAST_TYPE_SERIALIZER
