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
#include <boost/shared_ptr.hpp>


namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;

            namespace pimpl {
                class DataOutput;

                class DataInput;

                class SerializationContext;

                class ClassDefinition;

                class HAZELCAST_API PortableSerializer {
                public:

                    PortableSerializer(SerializationContext &serializationContext);

                    boost::shared_ptr<ClassDefinition> getClassDefinition(const Portable &p);

                    void write(DataOutput &dataOutput, const Portable &p);

                    void read(DataInput &dataInput, Portable &object, int factoryId, int classId, int dataVersion);

                private:
                    SerializationContext &context;

                };

            }
        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
