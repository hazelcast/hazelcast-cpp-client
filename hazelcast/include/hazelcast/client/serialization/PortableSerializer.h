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
#include "Serializer.h"
#include "Portable.h"

#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            class PortableSerializer{
            public:

                PortableSerializer(SerializationContext *const serializationContext);

                ~PortableSerializer();

                template <typename T>
                ClassDefinition *getClassDefinition(T& p) {
                    ClassDefinition *cd;
                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        p.writePortable(classDefinitionWriter);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, const T& p) {
                    ClassDefinition *cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, &dataOutput);
                    p.writePortable(portableWriter);
                };

                template <typename T>
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {

                    ClassDefinition *cd;
                    if (context->getVersion() == dataVersion) {
                        cd = context->lookup(factoryId, classId); // using serializationContext.version
                        PortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                    } else {
                        cd = context->lookup(factoryId, classId, dataVersion); // registered during read
                        MorphingPortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                    }
                };

            private:
                SerializationContext *const context;

            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
