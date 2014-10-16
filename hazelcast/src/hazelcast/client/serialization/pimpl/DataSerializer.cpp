//
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer() {

                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable &object) {
                    out.writeBoolean(true);
                    out.writeInt(object.getFactoryId());
                    out.writeInt(object.getClassId());
                    object.writeData(out);
                }

                void DataSerializer::read(ObjectDataInput &in, IdentifiedDataSerializable &object) {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read", " DataSerializable is not identified");
                    }
                    in.readInt(); //factoryId
                    in.readInt(); //classId
                    object.readData(in);
                }

            }
        }
    }
}


