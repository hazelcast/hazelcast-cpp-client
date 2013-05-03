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
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataInput;

            class DataOutput;

            class ClassDefinition;

            class SerializationContext;

            class SerializationService;

            class PortableFactory;

            class PortableSerializer {
            public:

                PortableSerializer(SerializationService *const serializationService, std::map < int, PortableFactory const * > const &portableFactories);

                ~PortableSerializer();

                boost::shared_ptr<ClassDefinition> getClassDefinition(Portable& p);

                int getTypeId();

                int getVersion();

                void write(DataOutput *output, Portable& p);

                std::auto_ptr<Portable> read(DataInput& dataInput);

                std::vector<int> const getFactoryIds() const;

            private:
                SerializationContext *const getSerializationContext();

                SerializationService *const service;
                std::map<int, PortableFactory const * > const &portableFactories;
            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
