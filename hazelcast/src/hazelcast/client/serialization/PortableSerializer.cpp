//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/PortableSerializer.h"
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/serialization/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/DefaultPortableReader.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableSerializer::PortableSerializer(SerializationContext &serializationContext)
            : context(serializationContext) {

            };

            ClassDefinition *PortableSerializer::getClassDefinition(const Portable &p) {
                ClassDefinition *cd;
                int factoryId = p.getFactoryId();
                int classId = p.getClassId();
                if (context.isClassDefinitionExists(factoryId, classId)) {
                    cd = context.lookup(factoryId, classId);
                } else {
                    ClassDefinitionWriter cdw(factoryId, classId, context.getVersion(), context);
                    PortableWriter classDefinitionWriter(&cdw);
                    p.writePortable(classDefinitionWriter);
                    cd = cdw.getClassDefinition();
                    cd = context.registerClassDefinition(cd);
                }

                return cd;
            };

            void PortableSerializer::write(DataOutput &dataOutput, const Portable &p) {
                ClassDefinition *cd = getClassDefinition(p);
                DefaultPortableWriter dpw(context, cd, dataOutput);
                PortableWriter portableWriter(&dpw);
                p.writePortable(portableWriter);
                portableWriter.end();
            };

            void PortableSerializer::read(DataInput &dataInput, Portable &object, int factoryId, int classId, int dataVersion) {
                ClassDefinition *cd;

                if (context.getVersion() == dataVersion) {
                    cd = context.lookup(factoryId, classId); // using serializationContext.version
                    if (cd == NULL) {
                        throw exception::IOException("PortableSerializer::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion)",
                                "Could not find class-definition for factory-id: " + util::to_string(factoryId) + ", class-id: " + util::to_string(classId) + ", version: " + util::to_string(dataVersion));
                    }
                    DefaultPortableReader defaultPortableReader(context, dataInput, cd);
                    PortableReader reader(&defaultPortableReader);
                    object.readPortable(reader);
                    reader.end();
                } else {
                    cd = context.lookup(factoryId, classId, dataVersion); // registered during read
                    if (cd == NULL) {
                        throw exception::IOException("PortableSerializer::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion)",
                                "Could not find class-definition for factory-id: " + util::to_string(factoryId) + ", class-id: " + util::to_string(classId) + ", version: " + util::to_string(dataVersion));
                    }
                    MorphingPortableReader morphingPortableReader(context, dataInput, cd);
                    PortableReader reader(&morphingPortableReader);
                    object.readPortable(reader);
                    reader.end();
                }
            };

        }
    }
}

