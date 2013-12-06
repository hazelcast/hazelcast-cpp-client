//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "hazelcast/util/HazelcastDll.h"
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

            class HAZELCAST_API PortableSerializer {
            public:

                PortableSerializer(SerializationContext& serializationContext);

                ClassDefinition *getClassDefinition(const Portable& p);

                void write(DataOutput &dataOutput, const Portable& p);

                void read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion);

            private:
                SerializationContext& context;

            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
