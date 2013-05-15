//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "ConstantSerializers.h"
#include "HazelcastException.h"

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
                    out << true;
                    out << getFactoryId(object);
                    out << getClassId(object);
                    writePortable(out, object);
                };

                template <typename T>
                void read(BufferedDataInput& in, T& object) {
                    bool identified = false;
                    in >> identified;
                    if (!identified) {
                        throw hazelcast::client::HazelcastException("void DataSerializer::read(BufferedDataInput& in, T& object) >  DataSerializable is not identified");
                    }
                    int factoryId = 0;
                    in >> factoryId;
                    int classId = 0;
                    in >> classId;
                    //TODO factoryId and classId is not used!!!
                    readPortable(in, object);
                };
            };
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER
