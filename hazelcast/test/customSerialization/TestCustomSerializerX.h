//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_XmlSerializer
#define HAZELCAST_XmlSerializer

#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/Serializer.h"
#include <assert.h>

namespace hazelcast {
    namespace client {
        namespace test {
            template<typename T>
            class HAZELCAST_API TestCustomSerializerX : public serialization::Serializer<T> {
            public:

                void write(serialization::ObjectDataOutput & out, const T& object) {
                    out.writeInt(666);
                    out.writeInt(object.id);
                    out.writeInt(666);
                }

                void read(serialization::ObjectDataInput & in, T& object) {
                    int i = in.readInt();
                    assert(i == 666);
                    object.id = in.readInt();
                    i = in.readInt();
                    assert(i == 666);
                }

                int getTypeId() const {
                    return 666;
                };
            };
        }
    }
}

#endif //HAZELCAST_XmlSerializer
