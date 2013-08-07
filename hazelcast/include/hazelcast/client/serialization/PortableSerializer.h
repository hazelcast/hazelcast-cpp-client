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
#include "ClassDefinitionWriter.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "MorphingPortableReader.h"
#include "Serializer.h"
#include "Portable.h"
#include "hazelcast/util/Util.h"
#include "DataInput.h"
#include "DataOutput.h"
#include <vector>
#include <map>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ObjectDataInput;

            class ObjectDataOutput;

            class PortableSerializer {
            public:

                PortableSerializer(SerializationContext& serializationContext);

                template <typename T>
                util::AtomicPointer<ClassDefinition> getClassDefinition(T& p) {
                    util::AtomicPointer<ClassDefinition> cd;
                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context.isClassDefinitionExists(factoryId, classId)) {
                        cd = context.lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context.getVersion(), context);
                        p.writePortable(classDefinitionWriter);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context.registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(DataOutput &dataOutput, const T& p) {
                    util::AtomicPointer<ClassDefinition> cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, dataOutput);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                };

                template <typename T>
                void read(DataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {
                    util::AtomicPointer<ClassDefinition> cd;

                    if (context.getVersion() == dataVersion) {
                        cd = context.lookup(factoryId, classId); // using serializationContext.version
                        PortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                        reader.end();
                    } else {
                        cd = context.lookup(factoryId, classId, dataVersion); // registered during read
                        MorphingPortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                        reader.end();
                    }
                };

            private:
                SerializationContext& context;

            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
