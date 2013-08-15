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
#include "DefaultPortableWriter.h"
#include "PortableWriter.h"
#include "DefaultPortableReader.h"
#include "PortableReader.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableSerializer::PortableSerializer(SerializationContext& serializationContext)
            : context(serializationContext){

            };

            util::AtomicPointer<ClassDefinition> PortableSerializer::getClassDefinition(const Portable& p) {
                util::AtomicPointer<ClassDefinition> cd;
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

            void PortableSerializer::write(DataOutput &dataOutput, const Portable& p) {
                util::AtomicPointer<ClassDefinition> cd = getClassDefinition(p);
                DefaultPortableWriter dpw(context, cd, dataOutput);
                PortableWriter portableWriter(&dpw);
                p.writePortable(portableWriter);
                portableWriter.end();
            };

            void PortableSerializer::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion) {
                util::AtomicPointer<ClassDefinition> cd;

                if (context.getVersion() == dataVersion) {
                    cd = context.lookup(factoryId, classId); // using serializationContext.version
                    DefaultPortableReader defaultPortableReader(context, dataInput, cd);
                    PortableReader reader(&defaultPortableReader);
                    object.readPortable(reader);
                    reader.end();
                } else {
                    cd = context.lookup(factoryId, classId, dataVersion); // registered during read
                    MorphingPortableReader morphingPortableReader(context, dataInput, cd);
                    PortableReader reader(&morphingPortableReader);
                    object.readPortable(reader);
                    reader.end();
                }
            };

        }
    }
}

