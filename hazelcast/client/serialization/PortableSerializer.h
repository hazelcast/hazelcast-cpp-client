//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "Portable.h"
#include <memory>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataInput;
            class DataOutput;
            class ClassDefinition;
            class SerializationContext;

            class PortableSerializer {
            public:
                PortableSerializer(SerializationContext*);
                ~PortableSerializer();

                boost::shared_ptr<ClassDefinition> getClassDefinition(Portable& p);

                int getTypeId();
                int getVersion();

                void write(DataOutput* output, Portable& p);

                std::auto_ptr<Portable> read(DataInput& dataInput);

            private:
                SerializationContext* context;
            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
