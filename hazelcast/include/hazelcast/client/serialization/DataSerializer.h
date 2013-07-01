//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_DATA_SERIALIZER
#define HAZELCAST_DATA_SERIALIZER

#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "ConstantSerializers.h"
#include "../HazelcastException.h"
#include "Serializer.h"
#include "DataSerializable.h"

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
                    writeData(out, object);
                };

                template <typename T>
                void read(BufferedDataInput& in, T& object) {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw HazelcastException("void DataSerializer::read(BufferedDataInput& in, T& object) >  DataSerializable is not identified");
                    }
                    in.readInt(); //factoryId
                    in.readInt(); //classId
                    //TODO factoryId and classId is not used!!!
                    readData(in, object);
                };

                ~DataSerializer();

            };
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER
