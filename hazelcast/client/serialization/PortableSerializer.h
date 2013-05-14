//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "SerializationContext.h"
#include "ClassDefinition.h"
#include "Util.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            class SerializationService;

            class PortableSerializer {
            public:

                PortableSerializer(SerializationService *const serializationService);

                ~PortableSerializer();

                template <typename T>
                boost::shared_ptr<ClassDefinition> getClassDefinition(T& p) {
                    boost::shared_ptr<ClassDefinition> cd;

                    SerializationContext *const context = getSerializationContext();
                    if (context->isClassDefinitionExists(getFactoryId(p), getClassId(p))) {
                        cd = context->lookup(getFactoryId(p), getClassId(p));
                    } else {
                        cd.reset(new ClassDefinition(getFactoryId(p), getClassId(p), context->getVersion()));
//                        PortableWriter classDefinitionWriter(this, cd, NULL, PortableWriter::CLASS_DEFINITION_WRITER);
//                        p.writePortable(classDefinitionWriter);
                        context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, T& p) {
                    boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
//                    PortableWriter writer(this, cd, &dataOutput, PortableWriter::DEFAULT);
//                    p.writePortable(writer);

                };

                template <typename T>
                std::auto_ptr<T> read(BufferedDataInput& dataInput, int factoryId, int dataClassId, int dataVersion) {

//                    PortableFactory const *portableFactory;
//                    if (portableFactories.count(factoryId) != 0) {
//                        portableFactory = portableFactories.at(factoryId);
//                    } else {
//                        throw hazelcast::client::HazelcastException("Could not find PortableFactory for factoryId: " + hazelcast::client::util::to_string(factoryId));
//                    }
//
//                    std::auto_ptr<Portable> p(portableFactory->create(dataClassId));
//                    if (p.get() == NULL) {
//                        throw hazelcast::client::HazelcastException("Could not create Portable for class-id: " + hazelcast::client::util::to_string(factoryId));
//                    }
                    std::auto_ptr<T> p(new T());
                    SerializationContext *const context = getSerializationContext();

                    boost::shared_ptr<ClassDefinition> cd;
                    if (context->getVersion() == dataVersion) {
                        cd = context->lookup(factoryId, dataClassId); // using context.version
//                        PortableReader reader(this, dataInput, cd, PortableReader::DEFAULT);
//                        p->readPortable(reader);
                    } else {
                        cd = context->lookup(factoryId, dataClassId, dataVersion); // registered during read
//                        PortableReader reader(this, dataInput, cd, PortableReader::MORPHING);
//                        p->readPortable(reader);
                    }
                    return p;
                };

                SerializationContext *const getSerializationContext();

            private:
                SerializationService *const service;
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
