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
#include "ClassDefinitionWriter.h"
#include "PortableWriter.h"
#import "PortableReader.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            class PortableSerializer {
            public:

                PortableSerializer(SerializationContext *const serializationContext);

                ~PortableSerializer();

                template <typename T>
                boost::shared_ptr<ClassDefinition> getClassDefinition(T& p) {
                    boost::shared_ptr<ClassDefinition> cd;

                    int factoryId = getFactoryId(p);
                    int classId = getClassId(p);
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        writePortable(classDefinitionWriter, p);
                        cd = classDefinitionWriter.getClassDefinition();
                        context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, T& p) {
                    boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, &dataOutput);
                    writePortable(portableWriter, p);
                };

                template <typename T>
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {

//                    PortableFactory const *portableFactory;
//                    if (portableFactories.count(factoryId) != 0) {
//                        portableFactory = portableFactories.at(factoryId);
//                    } else {
//                        throw hazelcast::client::HazelcastException("Could not find PortableFactory for factoryId: " + hazelcast::client::util::to_string(factoryId));
//                    }
//
//                    std::auto_ptr<Portable> p(portableFactory->create(classId));
//                    if (p.get() == NULL) {
//                        throw hazelcast::client::HazelcastException("Could not create Portable for class-id: " + hazelcast::client::util::to_string(factoryId));
//                    }

                    boost::shared_ptr<ClassDefinition> cd;
//                    if (context->getVersion() == dataVersion) {
                    cd = context->lookup(factoryId, classId); // using serializationContext.version
                    PortableReader reader(dataInput, cd);
                    readPortable(reader, object);
//                    } else {
//                        cd = context->lookup(factoryId, classId, dataVersion); // registered during read
//                        PortableReader reader(this, dataInput, cd, PortableReader::MORPHING);
//                        p->readPortable(reader);
//                    }
                };

            private:
                SerializationContext *const context;
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
