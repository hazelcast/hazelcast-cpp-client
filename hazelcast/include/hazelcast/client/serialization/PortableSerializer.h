//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "AtomicPointer.h"
#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {

        class Portable;

        namespace serialization {

            class DataOutput;

            class DataInput;

            class SerializationContext;

            class ClassDefinition;

            class PortableSerializer {
            public:

                PortableSerializer(SerializationContext& serializationContext);

                util::AtomicPointer<ClassDefinition> getClassDefinition(const Portable& p);

                void write(DataOutput &dataOutput, const Portable& p);

                void read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion);

            private:
                SerializationContext& context;

            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
