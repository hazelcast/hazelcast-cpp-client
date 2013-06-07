//
// Created by sancar koyunlu on 6/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TYPE_SERIALIZER
#define HAZELCAST_TYPE_SERIALIZER

namespace hazelcast {
    namespace client {
        namespace serialization {
            class TypeSerializer {
            public:
                virtual ~TypeSerializer() {

                };

//    virtual void write(BufferedDataOutput& out, T);
//    virtual T read(BufferedDataInput&);

                virtual int getTypeId() = 0;
            };
        }
    }
}


#endif //HAZELCAST_TYPE_SERIALIZER
