//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "ConstantSerializers.h"
#include "../HazelcastException.h"

#ifndef HAZELCAST_DATA_SERIALIZER
#define HAZELCAST_DATA_SERIALIZER

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataSerializer {
            public:
                DataSerializer();

                template <typename T>
                void write(BufferedDataOutput &out, T& object) {
                    out.writeBoolean(true);
                    out.writeInt(getFactoryId(object));
                    out.writeInt(getClassId(object));
                    writePortable(out, object);
                };

                template <typename T>
                void read(BufferedDataInput& in, T& object) {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw hazelcast::client::HazelcastException("void DataSerializer::read(BufferedDataInput& in, T& object) >  DataSerializable is not identified");
                    }
                    int factoryId = in.readInt();
                    int classId = in.readInt();
                    //TODO factoryId and classId is not used!!!
                    readPortable(in, object);
                };
            };
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER
