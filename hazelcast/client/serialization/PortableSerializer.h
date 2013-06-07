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
#include "../../util/Util.h"
#include "ClassDefinitionWriter.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "MorphingPortableReader.h"
#include "TypeSerializer.h"

#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            class PortableSerializer : public TypeSerializer {
            public:

                PortableSerializer(SerializationContext *const serializationContext);

                ~PortableSerializer();

                template <typename T>
                ClassDefinition *getClassDefinition(T& p) {
                    ClassDefinition *cd;

                    int factoryId = getFactoryId(p);
                    int classId = getClassId(p);
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        writePortable(classDefinitionWriter, p);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, T& p) {
                    ClassDefinition *cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, &dataOutput);
                    writePortable(portableWriter, p);
                };

                template <typename T>
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {
                    //TODO portable factory!!!!
//                    PortableFactory const *portableFactory;
//                    if (portableFactories.count(factoryId) != 0) {
//                        portableFactory = portableFactories.at(factoryId);
//                    } else {
//                        throw hazelcast::client::HazelcastException("Could not find PortableFactory for factoryId: " + hazelcast::util::to_string(factoryId));
//                    }
//
//                    std::auto_ptr<Portable> p(portableFactory->create(classId));
//                    if (p.get() == NULL) {
//                        throw hazelcast::client::HazelcastException("Could not create Portable for class-id: " + hazelcast::util::to_string(factoryId));
//                    }

                    ClassDefinition *cd;
                    if (context->getVersion() == dataVersion) {
                        cd = context->lookup(factoryId, classId); // using serializationContext.version
                        PortableReader reader(context, dataInput, cd);
                        hazelcast::client::serialization::readPortable(reader, object);
                    } else {
                        cd = context->lookup(factoryId, classId, dataVersion); // registered during read
                        MorphingPortableReader reader(context, dataInput, cd);
                        readPortable(reader, object);
                    }
                };

                int getTypeId();

            private:
                SerializationContext *const context;

            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
