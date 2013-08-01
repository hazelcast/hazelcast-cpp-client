//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_DATA_SERIALIZER
#define HAZELCAST_DATA_SERIALIZER

#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "IException.h"
#include "Serializer.h"
#include "DataSerializable.h"
#include "IOException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataSerializer {
            public:
                DataSerializer();

                template <typename T>
                void write(ObjectDataOutput &out, T& object) {
                    out.writeBoolean(true);
                    out.writeInt(object.getFactoryId());
                    out.writeInt(object.getClassId());
                    object.writeData(out);
                };

                template <typename T>
                void read(ObjectDataInput& in, T& object) {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::IOException("void DataSerializer::read", " DataSerializable is not identified");
                    }
                    in.readInt(); //factoryId
                    in.readInt(); //classId
                    //TODO factoryId and classId is not used!!!
                    object.readData(in);
                };

                ~DataSerializer();

            };
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER
